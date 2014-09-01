/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <time.h>
#include <string>
#include <list>
#include <map>
#include "pfc/log.h"
#include "unc/uppl_common.h"
#include "momgr_impl.hh"
#include "dhcprelay_server_momgr.hh"
#include "vlink_momgr.hh"
#include "vbr_flowfilter_entry_momgr.hh"
#include "flowlist_entry_momgr.hh"
#include "policingprofile_entry_momgr.hh"
#include "vrt_if_momgr.hh"
#include "vterm_if_momgr.hh"
#include "vbr_momgr.hh"
#include "vbr_if_momgr.hh"
#include "config_mgr.hh"
#include "vlanmap_momgr.hh"
#include "uncxx/upll_log.hh"

using unc::upll::ipc_util::IpcUtil;
using unc::upll::dal::DalResultCode;

namespace unc {
namespace upll {
namespace kt_momgr {
std::map<std::string, std::string> MoMgrImpl::auto_rename_;


#define KEY_TYPE_BIND_CS(key_type) ((key_type == UNC_KT_FLOWLIST) || \
                                    (key_type == UNC_KT_FLOWLIST_ENTRY) || \
                                    (key_type == UNC_KT_VRTIF_FLOWFILTER) || \
                                    (key_type == UNC_KT_VBR_FLOWFILTER) || \
                                    (key_type == UNC_KT_VBRIF_FLOWFILTER) || \
                                    (key_type == UNC_KT_POLICING_PROFILE) || \
                                    (key_type == UNC_KT_VTN_POLICINGMAP) || \
                                    (key_type == UNC_KT_VBR_POLICINGMAP) || \
                                    (key_type == UNC_KT_VBRIF_POLICINGMAP) || \
                                    (key_type == UNC_KT_VTERMIF_POLICINGMAP) || \
                                    (key_type == UNC_KT_VTN_FLOWFILTER))

upll_rc_t MoMgrImpl::CreateMo(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi) {
  if (ikey == NULL || req == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }

  switch (req->datatype) {
    case UPLL_DT_IMPORT:
    case UPLL_DT_CANDIDATE: {
      return CreateCandidateMo(req, ikey, dmi);
    }
    default:  {
      return UPLL_RC_ERR_GENERIC;
    }
  }
}

upll_rc_t MoMgrImpl::CreateImportMo(IpcReqRespHeader *req,
                                    ConfigKeyVal *ikey,
                                    DalDmlIntf *dmi,
                                    const char *ctrlr_id,
                                    const char *domain_id,
                                    upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || req == NULL) {
    return UPLL_RC_ERR_GENERIC;
  }
  switch (req->datatype) {
    case UPLL_DT_AUDIT: {
      return CreateAuditMoImpl(ikey, dmi, ctrlr_id);
    }
    case UPLL_DT_IMPORT: {
      if (!ctrlr_id) {
        UPLL_LOG_DEBUG("Empty controller name");
        return UPLL_RC_ERR_GENERIC;
      }
      upll_rc_t result_code = UPLL_RC_SUCCESS;
      ConfigKeyVal *temp_ckv = NULL;

      UPLL_LOG_TRACE("TODO Ctrlr Input %s", ikey->ToStr().c_str());

      if (UNC_KT_VBRIF_POLICINGMAP == ikey->get_key_type()) {
        val_policingmap_t *val_import = NULL;
        pfcdrv_val_vbrif_policingmap *pfc_val_import = NULL;
        UPLL_LOG_DEBUG("datatype (%d)", req->datatype);
        if (ikey->get_cfg_val() && (ikey->get_cfg_val()->get_st_num() ==
                                    IpctSt::kIpcStPfcdrvValVbrifPolicingmap)) {
          UPLL_LOG_TRACE("ikey = %s", ikey->ToStrAll().c_str());
          UPLL_LOG_TRACE("val struct num (%d)",ikey->get_cfg_val()->get_st_num());
          pfc_val_import = reinterpret_cast<pfcdrv_val_vbrif_policingmap *>(ikey->get_cfg_val()->get_val());
          if (pfc_val_import->valid[PFCDRV_IDX_VAL_POLICINGMAP_PM] == UNC_VF_VALID) {
            val_import = reinterpret_cast<val_policingmap_t *>
                (ConfigKeyVal::Malloc(sizeof(val_policingmap_t)));
            memcpy(val_import, &pfc_val_import->val_policing_map, sizeof(val_policingmap_t));
            UPLL_LOG_DEBUG("policer name (%s)", val_import->policer_name);
            ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStValPolicingmap, val_import));
          }
        } else {
          UPLL_LOG_DEBUG("vbr_if_policingmap Val struct NULL or Invalid val struct");
          return UPLL_RC_ERR_GENERIC;
        }
      } else if (UNC_KT_VBRIF_FLOWFILTER == ikey->get_key_type()) {
        if (ikey->get_cfg_val() &&
            (ikey->get_cfg_val()->get_st_num() ==
             IpctSt::kIpcStPfcdrvValVbrifVextif)) {
          UPLL_LOG_DEBUG("val struct num (%d)",ikey->get_cfg_val()->get_st_num());
          ikey->SetCfgVal(NULL);
        } else {
          UPLL_LOG_DEBUG("vbr_if_flowfilter Val struct NULL or Invalid val struct");
          return UPLL_RC_ERR_GENERIC;
        } 
      } else if (UNC_KT_VRTIF_FLOWFILTER == ikey->get_key_type()) {
        if (ikey->get_cfg_val() &&
            (ikey->get_cfg_val()->get_st_num() ==
             IpctSt::kIpcStPfcdrvValVbrifVextif)) {
          UPLL_LOG_DEBUG("val struct num (%d)",ikey->get_cfg_val()->get_st_num());
          ikey->SetCfgVal(NULL);
        } else {
          UPLL_LOG_DEBUG("vrt_if_flowfilter Val struct NULL or Invalid val struct");
          return UPLL_RC_ERR_GENERIC;
        }
      } else if ((UNC_KT_VBRIF_FLOWFILTER_ENTRY == ikey->get_key_type()) ||
                 (UNC_KT_VRTIF_FLOWFILTER_ENTRY == ikey->get_key_type())) {
        val_flowfilter_entry_t *val_ff_import = NULL;
        pfcdrv_val_flowfilter_entry_t *pfc_val_import = NULL;
        if (ikey->get_cfg_val() &&
            (ikey->get_cfg_val()->get_st_num() ==
             IpctSt::kIpcStPfcdrvValFlowfilterEntry)) {
          UPLL_LOG_DEBUG("val struct num (%d)",ikey->get_cfg_val()->get_st_num());
          pfc_val_import = reinterpret_cast<pfcdrv_val_flowfilter_entry_t *>
              (ikey->get_cfg_val()->get_val());
          val_ff_import = reinterpret_cast<val_flowfilter_entry_t *>
              (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_t)));
          memcpy(val_ff_import, &pfc_val_import->val_ff_entry, sizeof(val_flowfilter_entry_t));
          UPLL_LOG_DEBUG("FLOWLIST name (%s)", val_ff_import->flowlist_name);
          ikey->SetCfgVal(new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, val_ff_import));
        } else {
          UPLL_LOG_DEBUG("vbr_if_flowfilter/vrt_if_flowfilter Val struct NULL or Invalid val struct");
          return UPLL_RC_ERR_GENERIC;
        }
      } else {
        // nothing
      }

      result_code = ValidateMessage(req, ikey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("Validate Messgae Failed");
        return result_code;
      }
      if (UPLL_IMPORT_TYPE_PARTIAL == import_type) {
        bool is_rename = false;
        /*
         * VTN create will happen during vnode creates
         * in import
         * if VTN is stand alone in controller driver will
         * gives failure.
         */
        UPLL_LOG_TRACE("TODO Ctrlr Input %s", ikey->ToStr().c_str());
        if (UNC_KT_VTN == ikey->get_key_type())
          return UPLL_RC_SUCCESS;
        SET_USER_DATA_FLAGS(ikey, NO_RENAME);
        result_code = DupConfigKeyVal(temp_ckv, ikey);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
          return result_code;
        }
        /*
         * There is possbility of changing the vtn in some other domain
         */
        if (ISRENAME_KEYTYPE(ikey->get_key_type())) {
          UPLL_LOG_TRACE("TODO Rename key type info is %s", ikey->ToStr().c_str());
          if (VNODE_KEYTYPE (ikey->get_key_type())) {
            controller_domain ctrlr_dom;
            memset(&ctrlr_dom, 0, sizeof(controller_domain));
            result_code = GetControllerDomainId(ikey, &ctrlr_dom);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("GetControllerDomainId failed");
              delete temp_ckv;
              return result_code;
            }
            SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
            /*
             * Create a VTN in vnode creates, Because for
             * auto rename operation needs the domain name
             * while create an entry in to the rename table
             */
            bool is_vtn_renamed = false;
            result_code = CreateVtn(ikey, req->datatype, dmi, &is_vtn_renamed);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("CreateVtn failed %d", result_code);
              delete temp_ckv;
              return result_code;
            }
            /*
             * Gets the Unc Vtn and Unc Vnode name
             * There is possible new Vnode will come under the
             * reanmed VTN. So this function returns the UNC VTN
             * Name and Controller vnode name
             *
             * Nowhere touching ikey values,
             */
            result_code = GetUncKey(ikey, UPLL_DT_IMPORT, dmi,
                                    ctrlr_id);
            if (UPLL_RC_SUCCESS != result_code &&
                UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
              UPLL_LOG_DEBUG("GetUncKey failed %d", result_code);
              delete temp_ckv;
              return result_code;
            }
            UPLL_LOG_TRACE("The result code of GetUncKey %d", result_code);
            if (UPLL_RC_SUCCESS == result_code) {
              is_rename = true;
            }
          } else {
            /* For flowlist and Polcining profile checks
             * renamed or not
             */
            SET_USER_DATA_CTRLR(ikey, ctrlr_id);
            result_code = GetRenamedUncKey(ikey, req->datatype, dmi,
                                           (uint8_t *)ctrlr_id);
            if (UPLL_RC_SUCCESS != result_code &&
                UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
              UPLL_LOG_DEBUG("GetRenamedUncKey failed %d", result_code);
              delete temp_ckv;
              return result_code;
            }
            if (UPLL_RC_SUCCESS == result_code)
              is_rename = true;
          }
        }
        UPLL_LOG_DEBUG("Is rename flag value is %d", is_rename);
        if (!is_rename) {
          SET_USER_DATA_CTRLR(ikey, ctrlr_id);
          /*
           * This function is used to check auto rename required for
           * renaming key type. If required the generate auto rename
           * and set rename flag. Otherwise get the unc name if its 
           * renamed
           */
           if ((UNC_KT_VTN_FLOWFILTER == ikey->get_key_type()) ||
              (UNC_KT_VTN_FLOWFILTER_ENTRY == ikey->get_key_type()) ||
              (UNC_KT_VTN_POLICINGMAP == ikey->get_key_type())) {
             result_code = CreatePIForVtnPom(req, ikey, dmi, ctrlr_id);
             ikey->ResetWith(temp_ckv);
             delete temp_ckv;
             return result_code;
           } else {
             result_code = AutoRename(ikey, req->datatype,
                                      dmi, &is_rename);
             if (UPLL_RC_SUCCESS != result_code) {
               UPLL_LOG_DEBUG("AutoRename failed %d", result_code);
               delete temp_ckv;
               return result_code;
             }
           }
        }
      }
      result_code = CreateMo(req, ikey, dmi);
      if (UPLL_IMPORT_TYPE_PARTIAL == import_type) {
        /*
         * Convert UNC name into Ctrlr name if renamed
         */
        ikey->ResetWith(temp_ckv);
        delete temp_ckv;
      }
      return result_code;
    }
    default:
      return UPLL_RC_ERR_GENERIC;
  }
}

upll_rc_t MoMgrImpl::CreateCandidateMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || req == NULL) {
        return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // validate syntax and semantics
  if (UPLL_DT_IMPORT != req->datatype) {
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

  // Parent check
  if (ikey->get_key_type() == UNC_KT_VUNKNOWN) {
    // Vnode Existence check in CANDIDATE DB
    result_code = VnodeChecks(ikey, req->datatype, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(
          "Another Vnode with the same name already exists. Error code : %d",
           result_code);
      return result_code;
    }
  }

  /* In case of bypass vlink, domain info is required
   * so below code is needed */
  if (ikey->get_key_type() == UNC_KT_VUNK_IF) {
    ConfigKeyVal *okey = NULL;
    result_code = GetParentConfigKey(okey, ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error in retrieving the parent ConfigKeyVal");
      if (okey)
        delete okey;
      return UPLL_RC_ERR_GENERIC;
    }
    MoMgrImpl *vunk_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
        (const_cast<MoManager*>(GetMoManager(UNC_KT_VUNKNOWN))));
    DbSubOp dboper = { kOpReadSingle, kOpMatchNone,  kOpInOutDomain };
    result_code = vunk_mgr->ReadConfigDB(okey, req->datatype, UNC_OP_READ, dboper,
        dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Parent doesn't exist!");
      delete okey;
      okey = NULL;
      result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
        UPLL_RC_ERR_PARENT_DOES_NOT_EXIST: result_code;
      return result_code;
    }
    uint8_t *domain = NULL;
    GET_USER_DATA_DOMAIN(okey,domain);
    SET_USER_DATA_DOMAIN(ikey,domain);
    delete okey;
    okey = NULL;
  }
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_CREATE, dmi); 
  return result_code;
}

upll_rc_t MoMgrImpl::CreateAuditMoImpl(ConfigKeyVal *ikey,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  UPLL_LOG_TRACE(" ikey is %s", ikey->ToStrAll().c_str());
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
  UPLL_LOG_TRACE("ikey After GetRenamedUncKey %s", ikey->ToStrAll().c_str());

  result_code = SetValidAudit(ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT, UNC_OP_READ, dmi, MAINTBL);
  if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS && result_code !=
                     UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("UpdateConfigDB Failed err_code %d", result_code);
    return result_code;
  }
  result_code = (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) ? UPLL_RC_SUCCESS :
                                                           result_code;
  // Create a record in AUDIT DB
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    result_code = UpdateConfigDB(ikey, UPLL_DT_AUDIT, UNC_OP_CREATE, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("UpdateConfigDB Failed err_code %d", result_code);
    }
  }
  return result_code;
}

upll_rc_t MoMgrImpl::ReadMo(IpcReqRespHeader *header,
                            ConfigKeyVal *ikey,
                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ck_dhcp = NULL;
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));

  DhcpRelayServerMoMgr *mgr_dhcp = reinterpret_cast<DhcpRelayServerMoMgr *>
           (const_cast<MoManager *>(GetMoManager(UNC_KT_DHCPRELAY_SERVER)));
  if (!mgr_dhcp) {
    UPLL_LOG_DEBUG("Invalid Mgr");
    return UPLL_RC_ERR_GENERIC;
  }

  // To validate the read request from VTNService
  result_code = ValidateMessage(header, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed, result_code= %d",
                    result_code);
      return result_code;
  }

  if (header->option2 == UNC_OPT2_NEIGHBOR) {
    if (ikey->get_key_type() != UNC_KT_VUNK_IF) {
      return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
    }

    if ((header->operation == UNC_OP_READ) &&
        ((header->datatype == UPLL_DT_CANDIDATE) ||
         (header->datatype == UPLL_DT_RUNNING) ||
         (header->datatype == UPLL_DT_STARTUP) ||
         (header->datatype == UPLL_DT_STATE))) {
      result_code = PopulateValVtnNeighbor(ikey, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("PopulateValVtnNeighbor failed result_code %d", result_code);
      }
      return result_code;
    }
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }

  if ( ((header->option1 != UNC_OPT1_DETAIL) ||
        ((header->option1 == UNC_OPT1_DETAIL) &&
         (ikey->get_key_type() == UNC_KT_VBR_NWMONITOR)))&&
      header->option2 == UNC_OPT2_NONE) {
    result_code = ReadInfoFromDB(header, ikey, dmi, &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      /* VlanmapOnBoundary: vlanmap check added */
      if (ikey->get_key_type() == UNC_KT_VBR_VLANMAP) {
        upll_rc_t rc_t = AdaptValToVtnService(ikey,ADAPT_ALL);
        if (rc_t != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d", rc_t);
          return UPLL_RC_ERR_GENERIC;
        }
      }
      UPLL_LOG_DEBUG("ReadInfoFromDB failed, result_code= %d", result_code);
      return result_code;
    }
  }
  /* VlanmapOnBoundary: vlanmap check added */
  if (result_code == UPLL_RC_SUCCESS && (ikey->get_key_type() ==
                                         UNC_KT_VBR_VLANMAP)) {
    uint8_t user_flag = 0,vlanmap_flag =0;
    GET_USER_DATA_FLAGS(ikey, vlanmap_flag);
    user_flag = USER_VLANMAP_FLAG & vlanmap_flag;
    if (!user_flag) {
      upll_rc_t rc_t = AdaptValToVtnService(ikey,ADAPT_ALL);
      if (rc_t != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d", rc_t);
        return UPLL_RC_ERR_GENERIC;
      }

      UPLL_LOG_DEBUG("vlan map is not user configured by user");
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
  }

  if (header->datatype == UPLL_DT_STATE &&
      (header->option1 != UNC_OPT1_COUNT  &&
       header->option2 == UNC_OPT2_DHCP_RELAY)) {
    result_code = mgr_dhcp->GetVrtDhcpRelayServerAddress(ikey, dmi);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetVrtDhcpRelayServerAddress Failed");
      return result_code;
    }
  }
  if ((header->datatype == UPLL_DT_STATE) &&
      ((ikey->get_key_type() == UNC_KT_VBR_NWMONITOR) ||
       (ikey->get_key_type() == UNC_KT_DHCPRELAY_IF) ||
      ((header->option1 == UNC_OPT1_DETAIL) ||
       ((header->option2 != UNC_OPT2_DHCP_RELAY) &&
       (header->option2 != UNC_OPT2_NONE))))) {
    // check with karthi GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
    // Getting the controller vtn, vnode name
//  result_code = UpdateConfigDB(ikey, header->datatype, UNC_OP_READ, dmi);
//  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
//    UPLL_LOG_ERROR("Record Doesn't Exists in DB ");
//    return result_code;
//  }

    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain };

    if (ikey->get_key_type() != UNC_KT_VTN) {
      ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
      result_code = GetChildConfigKey(ck_dhcp, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetChildConfigKey is failed- %d", result_code);
        delete ck_dhcp;
        return result_code;
      }
      result_code = ReadConfigDB(ck_dhcp, header->datatype, UNC_OP_READ, dbop,
                                                                dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ReadConfigDB failed- %d", result_code);
        delete ck_dhcp;
        return result_code;
      }
      GET_USER_DATA_CTRLR_DOMAIN(ck_dhcp, ctrlr_dom);
      UPLL_LOG_TRACE("Controller Name is: %s", ctrlr_dom.ctrlr);
      result_code = ValidateCapability(
                header, ikey, reinterpret_cast<char *>(ctrlr_dom.ctrlr));

      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ValidateCapability is failed: %d", result_code);
        delete ck_dhcp;
        return result_code;
      }
      if (ikey->get_key_type() == UNC_KT_VLINK) {
        controller_domain ctrlr_vnode2_ctrlr;
        memset(&ctrlr_vnode2_ctrlr, 0, sizeof(controller_domain));
        GET_USER_DATA_CTRLR_DOMAIN(ck_dhcp->get_cfg_val(), ctrlr_vnode2_ctrlr);
        UPLL_LOG_INFO("2nd controller name %s",
          reinterpret_cast<const char *>(ctrlr_vnode2_ctrlr.ctrlr));
        if (strcmp((const char *)ctrlr_vnode2_ctrlr.ctrlr,
                                      (const char *)ctrlr_dom.ctrlr)) {
          result_code = ValidateCapability(header, ikey,
                        reinterpret_cast<const char *>(ctrlr_vnode2_ctrlr.ctrlr));
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ValidateCapability Failed for vlink %d", result_code);
            DELETE_IF_NOT_NULL(ck_dhcp);
            return result_code;
          }
        }
      }
    }
    result_code = GetRenamedControllerKey(ikey, header->datatype, dmi,
                                          &ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetRenamedControllerKey failed result_code %d",
                    result_code);
      DELETE_IF_NOT_NULL(ck_dhcp);
      return result_code;
    }
    UPLL_LOG_TRACE("After Read  %s", (ikey->ToStrAll()).c_str());

    IpcResponse ipc_resp;
    memset(&(ipc_resp), 0, sizeof(IpcResponse));
    IpcRequest ipc_req;
    memset(&ipc_req, 0, sizeof(ipc_req));
    memcpy(&(ipc_req.header), header, sizeof(IpcReqRespHeader));
    UPLL_LOG_TRACE("ConfigId %d", ipc_req.header.config_id);
    UPLL_LOG_TRACE("Operation %d", ipc_req.header.operation);
    UPLL_LOG_TRACE(" DataType %d", ipc_req.header.datatype);
    UPLL_LOG_TRACE("Option1 %d Option2 %d", ipc_req.header.option1, ipc_req.header.option2);
    ipc_req.ckv_data = ikey;
    UPLL_LOG_TRACE("Before Sending to Driver %s", (ikey->ToStrAll()).c_str());
    UPLL_LOG_TRACE("Domain Name %s", ctrlr_dom.domain);
    UPLL_LOG_TRACE("Controller Name %s", ctrlr_dom.ctrlr);

    if (!IpcUtil::SendReqToDriver((const char *)(ctrlr_dom.ctrlr),
           reinterpret_cast<char *>(ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
           PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_resp)) {
      UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                    ikey->get_key_type(), reinterpret_cast<char *>(ctrlr_dom.ctrlr));
      DELETE_IF_NOT_NULL(ck_dhcp);
      return ipc_resp.header.result_code;
    }

    UPLL_LOG_TRACE("AfterDriver Read from Controller %s", (ikey->ToStrAll()).c_str());
       // Populate ConfigKeyVal and IpcReqRespHeader with the response from driver
    if (ipc_resp.header.result_code == UPLL_RC_SUCCESS) {
      ikey->ResetWith(ipc_resp.ckv_data);
    }
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    result_code = ipc_resp.header.result_code;
    upll_rc_t child_result_code = GetChildConfigKey(ikey, ck_dhcp);
    if (UPLL_RC_SUCCESS != child_result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKeyFailed %d", child_result_code);
      DELETE_IF_NOT_NULL(ck_dhcp);
      return child_result_code;
    }
  }

  UPLL_LOG_TRACE("Before AdaptValtovtnservice  %s", (ikey->ToStrAll()).c_str());
  if (result_code == UPLL_RC_SUCCESS && UPLL_DT_IMPORT != header->datatype) {
    result_code = AdaptValToVtnService(ikey,ADAPT_ALL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d",
                    result_code);
      DELETE_IF_NOT_NULL(ck_dhcp);
      return result_code;
    }
  }
  if (ck_dhcp)
    delete ck_dhcp;

  return result_code;
}

upll_rc_t MoMgrImpl::ReadSiblingMo(IpcReqRespHeader *header,
                                   ConfigKeyVal *ikey,
                                   bool begin,
                                   DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;

  result_code = ValidateMessage(header, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                    result_code);
      return result_code;
  }

  ConfigKeyVal *tkey = NULL;
  ConfigKeyVal *start_key = ikey;

  bool set_flag = false;
  if (header->datatype == UPLL_DT_STATE && UNC_OPT2_NONE != header->option2) {
    if (ikey->get_cfg_val() == NULL) {
      set_flag = true;
      result_code = ReadInfoFromDB(header, ikey, dmi, &ctrlr_dom);
    } else {
      result_code = GetChildConfigKey(tkey, ikey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey Failed");
        return result_code;
      }
      result_code = ReadInfoFromDB(header, tkey, dmi, &ctrlr_dom);
      ConfigKeyVal *travel_key = tkey;
      while (travel_key) {
        ConfigVal *travel_val = ikey->get_cfg_val();
        ConfigVal *fval = (travel_val)->DupVal();
        travel_key->SetCfgVal(fval);
        while (travel_val) {
          travel_val = travel_val->get_next_cfg_val();
          if (travel_val) {
            ConfigVal *sval = travel_val->DupVal();
            travel_key->AppendCfgVal(sval);
          } else {
            break;
          }
        }
        travel_key = travel_key->get_next_cfg_key_val();
      }
      ikey =tkey;
    }
  } else {
    set_flag = true;
    result_code = ReadInfoFromDB(header, ikey, dmi, &ctrlr_dom);
  }

  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadInfoFromDB failed result_code %d",
                    result_code);
    DELETE_IF_NOT_NULL(tkey);
    return result_code;
  }

  if (header->datatype == UPLL_DT_STATE &&
     ((ikey->get_key_type() == UNC_KT_VBR_NWMONITOR) ||
      (ikey->get_key_type() == UNC_KT_DHCPRELAY_IF) ||
    header->option1 == UNC_OPT1_DETAIL ||
    header->option2 != UNC_OPT2_NONE)) {
    // ikey is retained in next_ckv, and then refered by end_resp_ckv. Once the
    // while loop is executed once, end_resp_ckv and ikey point to same.
    ConfigKeyVal *end_resp_ckv = NULL;
    ConfigKeyVal *one_ckv , *next_ckv;

    next_ckv = ikey;
    ConfigKeyVal *end_resp_ckv_tail = NULL;
    while (NULL != next_ckv) {
      one_ckv = next_ckv;
      next_ckv = next_ckv->get_next_cfg_key_val();
      one_ckv->set_next_cfg_key_val(NULL);
      GET_USER_DATA_CTRLR_DOMAIN(one_ckv, ctrlr_dom);
      // Get controller key
      // TODO(myself) GetReanmedControllerKey changes the actual data
      // and same will go to VTN Service. BUG.
      if (header->datatype == UPLL_DT_STATE &&
         (header->option1 != UNC_OPT1_COUNT &&
          header->option2 == UNC_OPT2_DHCP_RELAY)) {
        DhcpRelayServerMoMgr *mgr_dhcp =
               reinterpret_cast<DhcpRelayServerMoMgr *>
               (const_cast<MoManager *>(GetMoManager(UNC_KT_DHCPRELAY_SERVER)));
        if (!mgr_dhcp) {
          UPLL_LOG_DEBUG("Invalid Mgr");
          DELETE_IF_NOT_NULL(tkey);
          return UPLL_RC_ERR_GENERIC;
        }
        result_code = mgr_dhcp->GetVrtDhcpRelayServerAddress(one_ckv, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetVrtDhcpRelayServerAddress Failed");
          DELETE_IF_NOT_NULL(tkey);
          return result_code;
        }
        if (header->option1 == UNC_OPT1_NORMAL) {
          if (end_resp_ckv == NULL) {
            end_resp_ckv = ikey;
          } else {
            end_resp_ckv->AppendCfgKeyVal(one_ckv);
          }
          continue;
        }
      }

      DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutFlag
                      | kOpInOutDomain };
      ConfigKeyVal *temp_key = NULL;
      result_code = GetChildConfigKey(temp_key, one_ckv);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey Failed");
        DELETE_IF_NOT_NULL(tkey);
        return result_code;
      }
      result_code = ReadConfigDB(temp_key, header->datatype,
                                              UNC_OP_READ, dbop1, dmi, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
        DELETE_IF_NOT_NULL(temp_key);
        DELETE_IF_NOT_NULL(tkey);
        return result_code;
      }
      memset(&ctrlr_dom, 0, sizeof(controller_domain));
      GET_USER_DATA_CTRLR_DOMAIN(temp_key, ctrlr_dom);

      result_code = ValidateCapability(
            header, one_ckv, reinterpret_cast<const char *>
                                    (ctrlr_dom.ctrlr));
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ValidateCapability Failed %d", result_code);
        DELETE_IF_NOT_NULL(temp_key);
        DELETE_IF_NOT_NULL(tkey);
        return result_code;
      }
      if (ikey->get_key_type() == UNC_KT_VLINK) {
        controller_domain ctrlr_vnode2_ctrlr;
        memset(&ctrlr_vnode2_ctrlr, 0, sizeof(controller_domain));
        GET_USER_DATA_CTRLR_DOMAIN(temp_key->get_cfg_val(), ctrlr_vnode2_ctrlr);
        if (strcmp((const char *)ctrlr_vnode2_ctrlr.ctrlr,
                                      (const char *)ctrlr_dom.ctrlr)) {
          result_code = ValidateCapability(header, one_ckv,
                      reinterpret_cast<const char *>(ctrlr_vnode2_ctrlr.ctrlr));
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("ValidateCapability Failed %d", result_code);
            DELETE_IF_NOT_NULL(temp_key);
            DELETE_IF_NOT_NULL(tkey);
            return result_code;
          }
        }
      }
      result_code = GetRenamedControllerKey(one_ckv, header->datatype, dmi,
                                          &ctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetRenamedControllerKey failed result_code %d",
                        result_code);
        DELETE_IF_NOT_NULL(temp_key);
        DELETE_IF_NOT_NULL(tkey);
        return result_code;
      }
      if (UNC_KT_VBRIDGE == one_ckv->get_key_type() &&
          UNC_OPT2_NONE != header->option2) {
          UPLL_LOG_DEBUG("Set NULL for ConfigVal");
          one_ckv->SetCfgVal(NULL);
      }
      UPLL_LOG_TRACE("After Read  %s", (one_ckv->ToStrAll()).c_str());
      IpcResponse ipc_resp;
      memset(&(ipc_resp), 0, sizeof(IpcResponse));
      IpcRequest ipc_req;
      memset(&ipc_req, 0, sizeof(ipc_req));
      memcpy(&(ipc_req.header), header, sizeof(IpcReqRespHeader));
      ipc_req.header.operation = UNC_OP_READ;
      ipc_req.ckv_data = one_ckv;

      if (!IpcUtil::SendReqToDriver((const char *)(ctrlr_dom.ctrlr),
            reinterpret_cast<char *>(ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
            PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_resp)) {
        UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
           one_ckv->get_key_type(), reinterpret_cast<char *>(ctrlr_dom.ctrlr));
        DELETE_IF_NOT_NULL(temp_key);
        DELETE_IF_NOT_NULL(tkey);
        return ipc_resp.header.result_code;
      }
      UPLL_LOG_TRACE("AfterDriver Read from Controller %s",
                     (one_ckv->ToStrAll()).c_str());
      // Populate ConfigKeyVal with the response from driver
      if (ipc_resp.header.result_code == UPLL_RC_SUCCESS) {
        one_ckv->ResetWith(ipc_resp.ckv_data);
        delete ipc_resp.ckv_data;
      } else {
          DELETE_IF_NOT_NULL(temp_key);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          DELETE_IF_NOT_NULL(tkey);
          return ipc_resp.header.result_code;
      }
      if (end_resp_ckv == NULL) {
        end_resp_ckv = ikey;  // ikey and one_ckv should be same here
        if (one_ckv == ikey) {
          UPLL_LOG_TRACE("ikey has val");
        } else {
          UPLL_LOG_TRACE("ikey %p one_ckv %p", ikey, one_ckv);
        }
        end_resp_ckv_tail = end_resp_ckv;
      } else {
        end_resp_ckv_tail->AppendCfgKeyVal(one_ckv);
        end_resp_ckv_tail = one_ckv;
        UPLL_LOG_TRACE("ikey %p one_ckv %p end_ckv %p", ikey,
                       one_ckv, end_resp_ckv);
      }
      result_code = ipc_resp.header.result_code;
      upll_rc_t child_result_code = GetChildConfigKey(one_ckv, temp_key);
      if (UPLL_RC_SUCCESS != child_result_code) {
        DELETE_IF_NOT_NULL(temp_key);
        UPLL_LOG_DEBUG("GetChildConfigKey Failed %d", child_result_code);
        DELETE_IF_NOT_NULL(tkey);
        return child_result_code;
      }

      DELETE_IF_NOT_NULL(temp_key);
      UPLL_LOG_TRACE("AfterDriver Read from Controller %s",
                     (one_ckv->ToStrAll()).c_str());
    }

    UPLL_LOG_TRACE("BEFORE REST WITHr %s",
                     (end_resp_ckv->ToStrAll()).c_str());
    if (!set_flag && end_resp_ckv) {
      start_key->ResetWith(end_resp_ckv);
      delete end_resp_ckv;
    }
    UPLL_LOG_TRACE("AFTER REST WITHr %s",
                     (end_resp_ckv->ToStrAll()).c_str());
  }

  if (header->rep_count > 0) {
    if (result_code == UPLL_RC_SUCCESS
        && UPLL_DT_IMPORT != header->datatype) {
        result_code = AdaptValToVtnService(start_key, ADAPT_ALL);

      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d",
                    result_code);
        DELETE_IF_NOT_NULL(tkey);
        return result_code;
      }
    }
  }

  return result_code;
}

upll_rc_t MoMgrImpl::ReadSiblingCount(IpcReqRespHeader *header,
                                      ConfigKeyVal* ikey,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  // To validate the read sibling count request from VTNService
  result_code = ValidateMessage(header, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("ValidateMessage failed result_code %d",
                    result_code);
      return result_code;
  }
#if 0
  DbSubOp dbop1 = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutFlag
                   | kOpInOutDomain };

  ConfigKeyVal *temp_key = NULL;
  result_code = GetChildConfigKey(temp_key, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Failed");
    return result_code;
  }
  result_code = ReadConfigDB(temp_key, header->datatype,
                             UNC_OP_READ, dbop1, dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    return result_code;
  }
  GET_USER_DATA_CTRLR_DOMAIN(temp_key, ctrlr_dom);

  result_code = ValidateCapability(
         header, ikey, reinterpret_cast<const char *>
                                   (ctrlr_dom.ctrlr));
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateCapability Failed %d", result_code);
    return result_code;
  }
  if (ikey->get_key_type() == UNC_KT_VLINK) {
    controller_domain ctrlr_vnode2_ctrlr;
    memset(&ctrlr_vnode2_ctrlr, 0, sizeof(controller_domain));
    GET_USER_DATA_CTRLR_DOMAIN(temp_key->get_cfg_val(), ctrlr_vnode2_ctrlr);
    if (strcmp((const char *)ctrlr_vnode2_ctrlr.ctrlr,
                                    (const char *)ctrlr_dom.ctrlr)) {
      result_code = ValidateCapability(header, ikey,
                    reinterpret_cast<const char *>(ctrlr_vnode2_ctrlr.ctrlr));
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ValidateCapability Failed %d", result_code);
        return result_code;
      }
    }
  }
#endif
  result_code = ReadInfoFromDB(header, ikey, dmi, &ctrlr_dom);
  return result_code;
}

upll_rc_t MoMgrImpl::ReadInfoFromDB(IpcReqRespHeader *header,
                                         ConfigKeyVal* ikey,
                                         DalDmlIntf *dmi,
                                         controller_domain *ctrlr_dom) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  #if 0
  MoMgrTables tbl = (header->operation != UNC_OP_READ_SIBLING_COUNT &&
                     header->datatype == UPLL_DT_IMPORT) ? RENAMETBL:MAINTBL;
  #endif
  MoMgrTables tbl = MAINTBL;
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone,
                     kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
  // To validate the READ, READ_SIBLING, READ_SIBLING_BEGIN
  // and READ_SIBLING_COUNT message
  /* There is not difference between show running configuration
   * and show running configuration status in the request. so 
   * binding cs statuin in ruuing. Once the request is differentiate
   * have to modify here
   */
  if (UPLL_DT_RUNNING == header->datatype) {
    UPLL_LOG_TRACE("Status Flag enabled");
    dbop.inoutop |= kOpInOutCs;
  } else if(KEY_TYPE_BIND_CS(ikey->get_key_type())) {
    UPLL_LOG_TRACE("Status Flag enabled");
    dbop.inoutop |= kOpInOutCs;
  }

  switch (header->datatype) {
    case UPLL_DT_IMPORT:
    if (header->operation != UNC_OP_READ_SIBLING_COUNT) {
      result_code = ReadImportDB(ikey, header, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ReadImportDB failed %d", result_code);
        return result_code;
      }
      return result_code;
    }
    break;
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_RUNNING:
    case UPLL_DT_STARTUP:
    case UPLL_DT_STATE:
    {
      // To read the record(s) from DB
      upll_keytype_datatype_t dt_type =
          (ikey->get_key_type() == UNC_KT_VBR_NWMONITOR_HOST &&
          header->datatype == UPLL_DT_STATE) ?
          UPLL_DT_RUNNING : header->datatype;
      ConfigVal *cval = ikey->get_cfg_val();
      ConfigVal *new_cval = NULL;
      ConfigVal *orig_new_cval = NULL;
      if (cval) {
        while (cval) {
          UPLL_LOG_TRACE("Allocate new value struct...");
          AllocVal(new_cval, dt_type, MAINTBL);
          if (orig_new_cval == NULL)
            orig_new_cval = new_cval;
          const pfc_ipcstdef_t *st_def = IpctSt::GetIpcStdef(
                                            cval->get_st_num());
          UPLL_LOG_TRACE("Allocate new value struct for st_num:%d...", cval->get_st_num());
          memcpy(new_cval->get_val(), cval->get_val(), st_def->ist_size);
          if (dt_type == UPLL_DT_STATE) {
            cval = cval->get_next_cfg_val();
            new_cval = new_cval->get_next_cfg_val();
          } else {
            break;
          }
        }
        ikey->SetCfgVal(orig_new_cval);
      }
      if (header->operation == UNC_OP_READ) {
        result_code = ReadConfigDB(ikey, dt_type, header->operation,
                                   dbop, dmi, tbl);
      } else {
        if ((header->operation == UNC_OP_READ_SIBLING_BEGIN) ||
            (header->operation == UNC_OP_READ_SIBLING)) {
          dbop.readop = kOpReadMultiple;
        } else if (header->operation == UNC_OP_READ_SIBLING_COUNT) {
           dbop.readop = kOpReadCount;
        }
        UPLL_LOG_TRACE("Before the Read ConfigVal  is %s", (ikey->ToStrAll()).c_str());

        result_code = ReadConfigDB(ikey, dt_type, header->operation,
                                   dbop, header->rep_count, dmi, tbl);
        UPLL_LOG_TRACE("After the Read ConfigVal  is %s", (ikey->ToStrAll()).c_str());
      }
      break;
    }
    default:
      break;
  }
  if (result_code != UPLL_RC_SUCCESS)
    UPLL_LOG_DEBUG("Returning %d", result_code);
  return result_code;
}

upll_rc_t MoMgrImpl::ValidateDeleteMoReq(IpcReqRespHeader *req,
                                         ConfigKeyVal *ikey,
                                         DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // uint8_t rename = 0;

  if (!ikey || !req || !(ikey->get_key())) {
     UPLL_LOG_ERROR("Given Input is Empty")
     return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("Delete Operation for %d ", ikey->get_key_type());
  // cout << *ikey << "";
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Validate Message is Failed %d ", result_code);
    return result_code;
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_READ, dmi);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_ERROR("Record Doesn't Exists in DB ");
    return result_code;
  } else {
    result_code = UPLL_RC_SUCCESS;
  }
#if 1
  result_code = IsReferenced(ikey, req->datatype, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Semantic Error - IsReferenced ErrorCode: %d",
                     result_code);
      return result_code;  // Semantic Error
  }
#endif

  #if 0
  /* Check if vrouter admin status is disabled */
  result_code = IsAdminStatusEnable(ikey, dmi);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("IsAdminStatusEnable Failed %d ", result_code);
    return result_code;
  }
  #endif

  return result_code;
}


upll_rc_t MoMgrImpl::DeleteMo(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = ValidateDeleteMoReq(req, ikey, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("Returning Error - %d", result_code);
    return result_code;  // Semantic Error
  }
  result_code = DeleteCandidateMo(req, ikey, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("Returning Error - %d", result_code);
  }
  return result_code;
}


upll_rc_t MoMgrImpl::DeleteCandidateMo(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  bool fail = false;
#if 0
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  // uint8_t rename = 0;
  bool fail = false;

  if (!ikey || !req || !(ikey->get_key())) {
     UPLL_LOG_ERROR("Given Input is Empty")
     return result_code;
  }
  UPLL_LOG_TRACE("Delete Operation for %d ", ikey->get_key_type());
  // cout << *ikey << "";
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Validate Message is Failed %d ", result_code);
    return result_code;
  }
  result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE, UNC_OP_READ, dmi);
  if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
    UPLL_LOG_ERROR("Record Doesn't Exists in DB ");
    return result_code;
  }
  result_code = IsReferenced(ikey, req->datatype, dmi);
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Semantic Error - IsReferenced ");
      return result_code;  // Semantic Error
  }
#endif
  upll_rc_t result_code = DeleteChildren(ikey, ikey, req->datatype,
                                         dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("DeleteChildren %d", result_code);
      return result_code;  // Semantic Error
  }
  for (int i = 0; i < ntable; i++) {
    if (GetTable((MoMgrTables)i, UPLL_DT_CANDIDATE) >= uudst::kDalNumTables) {
      continue;
    }
    DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutNone};
    result_code = UpdateConfigDB(ikey, UPLL_DT_CANDIDATE,
             UNC_OP_DELETE, dmi, &dbop, (MoMgrTables)i);
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                  UPLL_RC_SUCCESS:result_code;
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Delete Operation fails with %d", result_code);
      fail = true;
    }
  }
  return ((fail == true)?UPLL_RC_ERR_GENERIC:UPLL_RC_SUCCESS);
}

upll_rc_t MoMgrImpl::UpdateMo(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;
  if (NULL == ikey || NULL == req || !(ikey->get_key())) {
     UPLL_LOG_ERROR("Given Input is Empty");
     return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("UpdateMo for %d", ikey->get_key_type());
  result_code = ValidateMessage(req, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_ERROR("Validation Message is Failed ");
      return result_code;
  }
  controller_domain ctrlr_dom;
  ctrlr_dom.ctrlr = ctrlr_dom.domain = NULL;
  ConfigKeyVal *temp_ck = NULL;
  result_code = GetChildConfigKey(temp_ck, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("Error in retrieving the Child ConfigKeyVal");
    if (temp_ck)
      delete temp_ck;
    return result_code;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain  };
  result_code = ReadConfigDB(temp_ck, req->datatype, UNC_OP_READ,
                             dbop, dmi, MAINTBL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ReadConfigDB Failed %d", result_code);
    delete temp_ck;
    return result_code;
  }

  /* Validate if all the attributes are UNC_VF_INVALID in the request. If all
   * INVALID, then return UPLL_RC_SUCCESS.
   * NOTE: However validation is not done here as below code finally does
   *       no-change update to database and return UPLL_RC_SUCCESS.
   */

  GET_USER_DATA_CTRLR_DOMAIN(temp_ck, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  if (ikey->get_key_type() != UNC_KT_VTN) {
    result_code = ValidateCapability(
                req, ikey, reinterpret_cast<char *>(ctrlr_dom.ctrlr));
    if (UPLL_RC_SUCCESS  != result_code) {
        UPLL_LOG_DEBUG("Validate Capability is Failed. Error_code : %d",
                      result_code);
      delete temp_ck;
      return result_code;
    }
  }
  delete temp_ck;
  // ValidateAttribute needs Controller Domain
  result_code = ValidateAttribute(ikey, dmi, req);
  if (UPLL_RC_SUCCESS  != result_code) {
      UPLL_LOG_ERROR("Validate Attribute is Failed");
      return result_code;
  }
  dbop.matchop = kOpMatchCtrlr | kOpMatchDomain;
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_UPDATE,
                               dmi, MAINTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_ERROR("Updation Failure in DB : %d", result_code);
    return result_code;
  }
  UPLL_LOG_TRACE("Updated Done Successfully %d", result_code);
  return result_code;
}

/*Return result of validation*/
upll_rc_t MoMgrImpl::TxVote(unc_key_type_t keytype,
                            DalDmlIntf *dmi,
                            ConfigKeyVal **err_ckv) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
}

/*Return result of validation*/
upll_rc_t MoMgrImpl::TxEnd(unc_key_type_t keytype, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_SUCCESS;
}

/*Invoked to parse the controller vote result*/
upll_rc_t MoMgrImpl::TxVoteCtrlrStatus(unc_key_type_t keytype,
                                       CtrlrVoteStatusList *ctrlr_vote_status,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t driver_result, result_code;
  driver_result = result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *err_key = NULL;
  uint8_t* ctrlr_id = NULL;
  CtrlrVoteStatusList::iterator cvsListItr;
  cvsListItr = ctrlr_vote_status->begin();
  CtrlrVoteStatus *cv_status_ptr;
  bool keytype_no_err = false;

  if (OVERLAY_KT(keytype))
    return UPLL_RC_SUCCESS;

  for (; cvsListItr != ctrlr_vote_status->end(); ++cvsListItr) {
    cv_status_ptr = *cvsListItr;
    // Retrieve the controler Id from the CtrlrVoteStatus object
    ctrlr_id = reinterpret_cast<uint8_t *>(
                    const_cast<char *>(cv_status_ptr->ctrlr_id.c_str()));
    UPLL_LOG_TRACE("TxVoteCtrlrStatus cvStatus controller ID: %s",
                  (cv_status_ptr->ctrlr_id).c_str());
    driver_result = (upll_rc_t) cv_status_ptr->upll_ctrlr_result;   \

    if (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) {
      UPLL_LOG_DEBUG(" vote status is controller disconnect");
      continue;
    }
    if (driver_result == UPLL_RC_SUCCESS) {
      UPLL_LOG_TRACE("TxVoteCtrlrStatus cvStatus net result: UPLL_RC_SUCCESS");
      continue;
    }
  // Retrieve the config Key as such from the KeyInfo
    for (err_key = cv_status_ptr->err_ckv; err_key;
               err_key = err_key->get_next_cfg_key_val()) {
      if (keytype != err_key->get_key_type())
        continue;
      keytype_no_err = true;
      // Perform RenameUncKey and get logical specific key values from db
      result_code = AdaptErrValToVtn(err_key, dmi, ctrlr_id, UPLL_DT_CANDIDATE);
      if (result_code != UPLL_RC_SUCCESS &&
          result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_TRACE("AdaptErrValToVtn failed %d", result_code);
        return result_code;
      }
      result_code = AdaptValToVtnService(err_key, ADAPT_ONE);
    }
  }
  UPLL_LOG_TRACE("TxVote Controller Status : %d", driver_result);
  if (!keytype_no_err)
    return UPLL_RC_SUCCESS;
  // Return vote Result to UPLLManager
  driver_result = (driver_result == UPLL_RC_ERR_CTR_DISCONNECTED) ?
                  UPLL_RC_SUCCESS : driver_result;
  // return driver_result;
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS : result_code;
  return result_code;
}

// To clear create/update flags in CANDIDATE database
upll_rc_t MoMgrImpl::TxClearCreateUpdateFlag(unc_key_type_t keytype,
                                             upll_keytype_datatype_t cfg_type,
                                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  // cfg_type should be CANDIDATE always since c_flag 
  // and u_flag exist only in CANDIDATE
  if (cfg_type != UPLL_DT_CANDIDATE) {
    UPLL_LOG_INFO("Invalid data type(%d)."
      "Create/update flags exist only in DT_CANDIDATE", cfg_type);
    return UPLL_RC_SUCCESS;
  }

  for (int tbl_index = (get_ntable()-1); tbl_index >= MAINTBL; tbl_index--) {
    uudst::kDalTableIndex dbtbl = GetTable((MoMgrTables)tbl_index,
                                           UPLL_DT_CANDIDATE);
    if (dbtbl >= uudst::kDalNumTables) {
      continue;
    }
    UPLL_LOG_TRACE("dbtbl:%d, key_type:%d", dbtbl, keytype);
    // At the end of commit operation, reset c_flag and u_flag 
    // in all the CANDIDATE tables
    DalResultCode db_result = dmi->ClearCreateUpdateFlags(dbtbl, cfg_type);

    upll_rc_t result_code = DalToUpllResCode(db_result);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_INFO("TxClearCreateUpdateFlags for tbl %d failed err code(%d,%d)",
                     dbtbl, db_result, result_code);
      return result_code;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::TxUpdateController(unc_key_type_t keytype,
                                        uint32_t session_id,
                                        uint32_t config_id,
                                        uuc::UpdateCtrlrPhase phase,
                                        set<string> *affected_ctrlr_set,
                                        DalDmlIntf *dmi,
                                        ConfigKeyVal **err_ckv)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  bool is_global_kt = false;
  IS_GLOBAL_KEYTYPE(keytype, is_global_kt);
  if (is_global_kt) {
    result_code = GlobalTxUpdateController(keytype, session_id, config_id,
                                           phase, affected_ctrlr_set, dmi, err_ckv);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_INFO("GlobalTxUpdateController failed for keytype %d  %d",
                    keytype, result_code);
    } else {
      UPLL_LOG_TRACE("GlobalTxUpdateController returned %d", result_code);
    }
    return result_code;
  }
  DalResultCode db_result;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL, *ck_old = NULL;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle = NULL;
  IpcResponse ipc_resp;
  if (affected_ctrlr_set == NULL)
    return UPLL_RC_ERR_GENERIC;
  // Get the corresponding operation (CREATE/UPDATE/DELETE) based on the phase
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
      ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
       ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));
  if (op == UNC_OP_INVALID) {
    UPLL_LOG_INFO("Invalid operation received-%d", op);
    // Not a valid operation
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  // Update operation not supported fot UNC_KT_VBR_FLOWFILTER
  if ((UNC_OP_UPDATE == op) && (UNC_KT_VBR_FLOWFILTER == keytype)) {
    UPLL_LOG_DEBUG("Update operation not supported for keytype-%d", keytype);
    return UPLL_RC_SUCCESS;
  }
  // Get CREATE, DELETE and UPDATE object information from the MAINTBL between
  // candidate configuration and running configuration where 'req' parameter
  // contains the candidate information and
  // the 'nreq' parameter contains the running configuration.
  result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
                             op, req, nreq, &dal_cursor_handle, dmi, MAINTBL);
  while (result_code == UPLL_RC_SUCCESS) {
    // Iterate loop to get next record
    db_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(db_result);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" GetNextRecord failed err code(%d)", result_code);
      break;
    }
    // Duplicates the candidate ConfigKeyVal 'req' into 'ck_main'
    result_code = DupConfigKeyVal(ck_main, req);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("DupConfigKeyVal failed for candidate ConfigKeyVal-%d",
                    result_code);
      dmi->CloseCursor(dal_cursor_handle, true);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      return result_code;
    }
    if (op == UNC_OP_UPDATE) {
      // Duplicates the running ConfigKeyVal 'nreq' into 'ck_old'
      result_code = DupConfigKeyVal(ck_old, nreq);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("DupConfigKeyVal failed  for running ConfigKeyVal-%d",
                      result_code);
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(ck_main);
        return result_code;
      }
    }
    // Case1: commit(del) - Check in RUNNING since no info exists in CAND
    // Case2: Commit(Cr/upd) - Check in CANDIDATE always
    upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
        UPLL_DT_RUNNING : UPLL_DT_CANDIDATE;
    if ((op == UNC_OP_CREATE) || (op == UNC_OP_UPDATE)) {
      void *main = GetVal(ck_main);
      void *val_nrec = (ck_old) ? GetVal(ck_old) : NULL;
      // Filter the attribute which is not sent to controller
      if (FilterAttributes(main, val_nrec, false, op)) {
        DELETE_IF_NOT_NULL(ck_main);
        DELETE_IF_NOT_NULL(ck_old);
        continue;
      }
    }
    // not_send_to_drv is to decide whether the configuration needs
    // to be sent to controller or not
    bool not_send_to_drv = false;
    // Perform validation on key type specific, before sending to driver
    result_code = AdaptValToDriver(ck_main, ck_old, op,
                                   dt_type, keytype, dmi, not_send_to_drv, false);
    if (result_code != UPLL_RC_SUCCESS) {
      if (result_code == UPLL_RC_ERR_CFG_SEMANTIC) {
        UPLL_LOG_INFO("AdaptValToDriver SemanticValidation"
                      " failed %d", result_code);
        // To convert the value structure read from DB to VTNService during
        // CREATE/DELETE/UPDATE operations
        if (keytype == UNC_KT_VBR_IF || keytype  == UNC_KT_VBR_VLANMAP) {
          upll_rc_t temp_rc = AdaptValToVtnService(ck_main, ADAPT_ONE);
          if (temp_rc != UPLL_RC_SUCCESS &&
              temp_rc != UPLL_RC_ERR_GENERIC) {
            UPLL_LOG_DEBUG("AdaptValToVtnService failed %d\n", result_code);
          }
        }
        *err_ckv = ck_main;
      } else {
        UPLL_LOG_INFO("AdaptValToDriver failed %d", result_code);
        DELETE_IF_NOT_NULL(ck_main);
      }
      DELETE_IF_NOT_NULL(ck_old);
      DELETE_IF_NOT_NULL(req);
      DELETE_IF_NOT_NULL(nreq);
      dmi->CloseCursor(dal_cursor_handle, true);
      return result_code;
    }
    if (not_send_to_drv) {
      UPLL_LOG_TRACE("%s Configuration is not sent to controller",
                     (ck_main->ToStr()).c_str());
      DELETE_IF_NOT_NULL(ck_main);
      DELETE_IF_NOT_NULL(ck_old);
      continue;
    }
    // Get controller & domain-id from ConfigKeyVal 'ck_main'
    GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
    uint8_t flags = 0;
    // GET_USER_DATA_FLAGS determines whether the vlanMap is a user
    // configured vlanMap or a boundary vlanMap
    GET_USER_DATA_FLAGS(ck_main, flags);
    // If operation is UPDATE, append the running ConfigVal to the
    // candidate ConfigKeyVal using AppendCfgVal().
    // This provides the old and new value structures to the
    // driver in the case of UPDATE operation.
    if (op == UNC_OP_UPDATE) {
      ConfigVal *next_val = (ck_old->get_cfg_val())->DupVal();
      uint8_t rename_flag = 0;
      GET_USER_DATA_FLAGS(ck_old->get_cfg_val(), rename_flag)
      SET_USER_DATA_FLAGS(next_val, rename_flag)
      ck_main->AppendCfgVal(next_val);
    }

    if (!OVERLAY_KT(keytype)) {
      // Get the controller key for the renamed unc key.
      result_code = GetRenamedControllerKey(ck_main, dt_type,
                                            dmi, &ctrlr_dom);

      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" GetRenamedControllerKey failed err code(%d)",
                       result_code);
        DELETE_IF_NOT_NULL(ck_main);
        DELETE_IF_NOT_NULL(ck_old);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        dmi->CloseCursor(dal_cursor_handle, true);
        return result_code;
      }
    }
    UPLL_LOG_TRACE("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                   ctrlr_dom.domain);
    // Inserting the controller to Set
    affected_ctrlr_set->insert
        (string(reinterpret_cast<char *>(ctrlr_dom.ctrlr)));
    // To populate the Ipc Request
    result_code = SendIpcReq(session_id, config_id, op, UPLL_DT_CANDIDATE,
                             ck_main, &ctrlr_dom, &ipc_resp);

    if (result_code == UPLL_RC_ERR_CTR_DISCONNECTED) {
      UPLL_LOG_DEBUG(" driver result code - %d", ipc_resp.header.result_code);
      result_code = UPLL_RC_SUCCESS;
    }
    if (result_code != UPLL_RC_SUCCESS) {
      // If Driver returned error, return the err_ckv received from driver after
      // getting its UNC key and apdapting the CKV to North Bound API
      // If update to driver failed, exit the loop.
      UPLL_LOG_DEBUG("IpcSend failed %d", result_code);

      /* VlanmapOnBoundary: Added vlanmap check */
      upll_rc_t local_rc = UPLL_RC_SUCCESS;
      if (ck_main->get_key_type() == UNC_KT_VBR_VLANMAP) {
        VlanMapMoMgr *vlanmap_mgr = reinterpret_cast<VlanMapMoMgr *>(
            const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_VLANMAP)));

//        SET_USER_DATA_FLAGS(ck_main, flags);

        ConfigKeyVal *err = NULL;

        local_rc = vlanmap_mgr->TranslateError(&err, req, dmi, dt_type);
        if (local_rc != UPLL_RC_SUCCESS) {
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ck_old);
          UPLL_LOG_ERROR("Failed to convert vlanmap error ckv");
          return local_rc;
        }

        if (flags & BOUNDARY_VLANMAP_FLAG) {
          SET_USER_DATA_CTRLR(err, ctrlr_dom.ctrlr);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          *err_ckv = err;
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ck_old);
          break;
        } else {
          DELETE_IF_NOT_NULL(err);
        }
      }
      if (ipc_resp.ckv_data) {
        if (ck_main->get_key_type() == UNC_KT_VBR_IF) {
          ConfigKeyVal *ck_vlink = NULL;
          /* Validating if VbridgeIf is a node for Vlink */

          VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>
              (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
          vn_if_type iftype;
          if (!mgr) {
            UPLL_LOG_DEBUG("Invalid mgr");
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            DELETE_IF_NOT_NULL(ck_old);
            DELETE_IF_NOT_NULL(ck_main);
            dmi->CloseCursor(dal_cursor_handle, true);
            DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
            return UPLL_RC_ERR_GENERIC;
          }
          ConfigKeyVal *ck_vif = NULL;
          local_rc = GetChildConfigKey(ck_vif,req);

          local_rc = mgr->CheckIfMemberOfVlink(ck_vif,
                                    UPLL_DT_CANDIDATE, ck_vlink, dmi, iftype);
          if (local_rc == UPLL_RC_SUCCESS) {
            DELETE_IF_NOT_NULL(ck_vif);
            delete ipc_resp.ckv_data;
            SET_USER_DATA_CTRLR(ck_vlink, ctrlr_dom.ctrlr);
            *err_ckv = ck_vlink;
            DELETE_IF_NOT_NULL(ck_old);
            DELETE_IF_NOT_NULL(ck_main);
            break;
          } else if(local_rc == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_TRACE("Checking in running table");
            local_rc = mgr->CheckIfMemberOfVlink(ck_vif,
                                     UPLL_DT_RUNNING, ck_vlink, dmi, iftype);
            DELETE_IF_NOT_NULL(ck_vif);
            if (local_rc == UPLL_RC_SUCCESS) {
              delete ipc_resp.ckv_data;
              SET_USER_DATA_CTRLR(ck_vlink, ctrlr_dom.ctrlr);
              *err_ckv = ck_vlink;
              DELETE_IF_NOT_NULL(ck_old);
              DELETE_IF_NOT_NULL(ck_main);
              break;
            } else {
              SET_USER_DATA_CTRLR(ipc_resp.ckv_data, ctrlr_dom.ctrlr);
              *err_ckv = ipc_resp.ckv_data;
              if (ck_vlink) delete ck_vlink;
              UPLL_LOG_DEBUG("Failed to map boundary if to vlink");
            }
          }
        }
        // Get the UNC key for the renamed controller key.
        local_rc = GetRenamedUncKey(ipc_resp.ckv_data, dt_type, dmi,
                                    ctrlr_dom.ctrlr);
        if (UPLL_RC_SUCCESS != local_rc &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != local_rc) {
          UPLL_LOG_DEBUG("GetRenamedUncKey failed %d", local_rc);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ck_old);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          result_code = UPLL_RC_ERR_GENERIC;
          break;
        }
        // To convert the value structure read from DB to
        // VTNService during READ operations
        local_rc = AdaptValToVtnService(ipc_resp.ckv_data,ADAPT_ONE);
        if (local_rc != UPLL_RC_SUCCESS &&
            local_rc != UPLL_RC_ERR_GENERIC) {
          // If no val structure, ignore error
          UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d",
                         local_rc);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ck_old);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
          result_code = UPLL_RC_ERR_GENERIC;
          break;
        }
        SET_USER_DATA_CTRLR(ipc_resp.ckv_data, ctrlr_dom.ctrlr);
        *err_ckv = ipc_resp.ckv_data;
      }
      DELETE_IF_NOT_NULL(ck_main);
      DELETE_IF_NOT_NULL(ck_old);
      break;
    }
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    DELETE_IF_NOT_NULL(ck_old);
    DELETE_IF_NOT_NULL(ck_main);
  }
  if (dal_cursor_handle)
    dmi->CloseCursor(dal_cursor_handle, true);
  DELETE_IF_NOT_NULL(req);
  DELETE_IF_NOT_NULL(nreq);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
      UPLL_RC_SUCCESS : result_code;
  return result_code;
}

upll_rc_t MoMgrImpl::SendIpcReq(uint32_t session_id,
                               uint32_t config_id,
                               unc_keytype_operation_t op,
                               upll_keytype_datatype_t dt_type,
                               ConfigKeyVal *&ckv,
                               controller_domain *ctrlr_dom,
                               IpcResponse *ipc_resp) {
  UPLL_FUNC_TRACE;
  upll_rc_t rc = UPLL_RC_SUCCESS;
  if (!ipc_resp)
    return UPLL_RC_ERR_GENERIC;
  memset(ipc_resp, 0, sizeof(IpcResponse));
  IpcRequest ipc_req;
  memset(&ipc_req, 0, sizeof(ipc_req));
  ipc_req.header.clnt_sess_id = session_id;
  ipc_req.header.config_id = config_id;
  ipc_req.header.operation = op;
  ipc_req.header.datatype = dt_type;
  ipc_req.ckv_data = ckv;

  UPLL_LOG_TRACE(" session id - %d, configuration id - %d",
                 session_id, config_id);

  if (ctrlr_dom == NULL) {
    /* VlanmapOnBoundary: To get the boundary name 
     * associated with the logical_port_id */
    if (ckv->get_key_type() == UNC_KT_BOUNDARY && op == UNC_OP_READ_SIBLING) {
      ipc_req.header.rep_count = 10000;
    }
    if (!uui::IpcUtil::SendReqToPhysical(UPPL_IPC_SVC_NAME, UPPL_SVC_READREQ,
                                         &ipc_req, ipc_resp)) {
      UPLL_LOG_INFO("Send Request to physical for Key %d failed ", ckv->get_key_type());
    }
  } else {
    if (!IpcUtil::SendReqToDriver((const char *)ctrlr_dom->ctrlr,
                                  reinterpret_cast<char *>(ctrlr_dom->domain),
                                  NULL,
                                  static_cast<pfc_ipcid_t>(0),
                                  &ipc_req, true, ipc_resp)) {
      UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                    ckv->get_key_type(), reinterpret_cast<char *>(ctrlr_dom->ctrlr));
    }
  }
  rc = ipc_resp->header.result_code;
  /* ipc_util prints
  if (rc != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Request for Key %d failed  with error %d",
                  ckv->get_key_type(), rc);
  }
  */
  return rc;
}



upll_rc_t MoMgrImpl::TxCopyCandidateToRunning(
                                    unc_key_type_t key_type,
                                    CtrlrCommitStatusList *ctrlr_commit_status,
                                    DalDmlIntf* dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_keytype_operation_t op[] = {UNC_OP_DELETE, UNC_OP_CREATE, UNC_OP_UPDATE};
  ConfigKeyVal *req = NULL, *nreq = NULL, *ckey = NULL;
  DalCursor *cfg1_cursor;
  DalResultCode db_result = uud::kDalRcSuccess;
  int nop = sizeof(op)/ sizeof(op[0]);
  uint8_t *ctrlr_id = NULL;
  map<string, int> ctrlr_result;
  CtrlrCommitStatusList::iterator ccsListItr;
  CtrlrCommitStatus *ccStatusPtr;

  if (ctrlr_commit_status != NULL) {
  for (ccsListItr = ctrlr_commit_status->begin();
        ccsListItr != ctrlr_commit_status->end(); ++ccsListItr) {
      ccStatusPtr = *ccsListItr;
      ctrlr_id =
        reinterpret_cast<uint8_t *>(
            const_cast<char *>(ccStatusPtr->ctrlr_id.c_str()));
      ctrlr_result[ccStatusPtr->ctrlr_id] = ccStatusPtr->upll_ctrlr_result;
      if (ccStatusPtr->upll_ctrlr_result != UPLL_RC_SUCCESS) {
        for (ConfigKeyVal *ck_err = ccStatusPtr->err_ckv; ck_err != NULL;
            ck_err = ck_err->get_next_cfg_key_val()) {
          if (ck_err->get_key_type() != key_type) continue;
          if (!OVERLAY_KT(key_type)) {
            result_code = AdaptErrValToVtn(ck_err, dmi, ctrlr_id,
                                           UPLL_DT_CANDIDATE);
            if (result_code != UPLL_RC_SUCCESS &&
                result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
              UPLL_LOG_TRACE("AdaptErrValToVtn failed %d", result_code);
              return result_code;
            }
          }
          result_code = AdaptValToVtnService(ck_err, ADAPT_ONE);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("Returning error %d\n", result_code);
            return result_code;
          }
        }
      }
    }
  }
  for (int i = 0; i < nop; i++)  {
    result_code= DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op[i],
          req, nreq, &cfg1_cursor, dmi, NULL, MAINTBL, true);
    while (result_code == UPLL_RC_SUCCESS) {
      db_result = dmi->GetNextRecord(cfg1_cursor);
      result_code = DalToUpllResCode(db_result);
      if (result_code != UPLL_RC_SUCCESS)
        break;
      if (op[i] == UNC_OP_DELETE)
        result_code = GetChildConfigKey(ckey, req);
      else
        result_code = DupConfigKeyVal(ckey, req, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS)
        break;
      string controller;
      uint32_t driver_result;
      switch (ckey->get_key_type()) {
      case UNC_KT_VUNKNOWN:
      case UNC_KT_VUNK_IF:
        driver_result = UPLL_RC_SUCCESS;
        break;
      default:
        GET_USER_DATA_CTRLR(ckey, ctrlr_id);
        controller = reinterpret_cast<char *>(ctrlr_id);
        if (ctrlr_result.empty()) {
          UPLL_LOG_TRACE("ctrlr_commit_status is NULL");
          driver_result = UPLL_RC_ERR_CTR_DISCONNECTED;
        } else {
          driver_result = ctrlr_result[controller];
        }
      }
      upll_keytype_datatype_t dt_type = (op[i] == UNC_OP_CREATE)? UPLL_DT_STATE:UPLL_DT_RUNNING;
      if (op[i] != UNC_OP_DELETE) {
         if (key_type == UNC_KT_VLINK) {
           uint8_t *ctrlr_id2 = NULL;
           GET_USER_DATA_CTRLR(ckey->get_cfg_val(), ctrlr_id2);
           if (ctrlr_id2 && strncmp(reinterpret_cast<const char *>(ctrlr_id), 
                                    reinterpret_cast<const char *>(ctrlr_id2),
                                    kMaxLenCtrlrId+1)) {
             string controller2(reinterpret_cast<char *>(ctrlr_id2));
             if (ctrlr_result.empty()) {
               UPLL_LOG_TRACE("ctrlr_commit_status is NULL");
               UpdateConfigStatus(ckey, op[i], UPLL_RC_ERR_CTR_DISCONNECTED, nreq, dmi);
             } else {
               uint32_t cons_result = ctrlr_result[controller2] |
                                      ctrlr_result[controller];
               UpdateConfigStatus(ckey, op[i], cons_result, nreq, dmi);
             }
           }  else {
             if (ctrlr_result.empty()) {
               UPLL_LOG_TRACE("ctrlr_commit_status is NULL");
               UpdateConfigStatus(ckey, op[i], UPLL_RC_ERR_CTR_DISCONNECTED,
                                  nreq, dmi);
             } else {
             UpdateConfigStatus(ckey, op[i], ctrlr_result[controller],
                                nreq, dmi);
             }
           }
         } else if ((op[i] == UNC_OP_UPDATE) && VN_IF_KEYTYPE(key_type)) {
           UpdateConfigStatus(ckey, op[i], driver_result, nreq, dmi);
         } else {
           UpdateConfigStatus(ckey, op[i], driver_result, nreq, dmi);
         }
         /* if portmapping / admin status on kt is updated,
          *  oper status has to be reset*/
         if (GetStateVal(ckey))
           dt_type = UPLL_DT_STATE;
      } else {
         if (OperStatusSupported(ckey->get_key_type())) {
           result_code = UpdateParentOperStatus(ckey,dmi, driver_result);
           if (result_code != UPLL_RC_SUCCESS) {
             delete ckey;
             DELETE_IF_NOT_NULL(req);
             DELETE_IF_NOT_NULL(nreq);
             if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
             UPLL_LOG_DEBUG("Returning error %d\n",result_code);
             return result_code;
           }
         }
      }
      if(!(((ckey->get_key_type() == UNC_KT_VBR_IF) ||
            (ckey->get_key_type() == UNC_KT_VRT_IF) ||
            (ckey->get_key_type() == UNC_KT_VTERM_IF)) &&
          ((op[i] == UNC_OP_CREATE) || (op[i] == UNC_OP_UPDATE)))) {
        DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutCs |
                               kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain};
        result_code = UpdateConfigDB(ckey, dt_type, op[i], dmi, &dbop_update,
                                                                  MAINTBL);
      }
      if (result_code != UPLL_RC_SUCCESS) {
        delete ckey;
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        if (cfg1_cursor)
          dmi->CloseCursor(cfg1_cursor, true);
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        return UPLL_RC_ERR_GENERIC;
      }
      EnqueCfgNotification(op[i], UPLL_DT_RUNNING, ckey);
      delete ckey;
      ckey = NULL;
    }
    if (req)
      delete req;
    if (nreq)
      delete nreq;
    req = nreq = NULL;
    if (cfg1_cursor) {
      dmi->CloseCursor(cfg1_cursor, true);
      cfg1_cursor = NULL;
    }
    // Copying Rename Table to Running
    UPLL_LOG_TRACE("keytype is %d", key_type);
    if ((key_type == UNC_KT_VBRIDGE) ||
        (key_type == UNC_KT_VLINK)) {
      result_code = TxCopyRenameTableFromCandidateToRunning(key_type,
                                                            op[i], dmi);
      UPLL_LOG_DEBUG("TxCopyRenameTableFromCandidateToRunning returned %d",
                                                            result_code);
    }
  }
  if (!ctrlr_result.empty())
    ctrlr_result.clear();
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
                 UPLL_RC_SUCCESS: result_code;
  return result_code;
}

upll_rc_t MoMgrImpl::EnqueCfgNotification(unc_keytype_operation_t op,
                                          upll_keytype_datatype_t dt_type,
                                          ConfigKeyVal *ctrlr_key) {
  UPLL_FUNC_TRACE;
  ConfigNotification *ck_notify;
  unc_keytype_datatype_t unc_type = (unc_keytype_datatype_t)dt_type;
  ConfigKeyVal *okey = NULL;
  upll_rc_t result_code = GetChildConfigKey(okey, ctrlr_key);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Returning error %d", result_code);
    return result_code;
  }
  ck_notify = new ConfigNotification(op, unc_type, okey);
  if (!(ConfigNotifier::BufferNotificationToUpllUser(ck_notify))) {
    delete ck_notify;
  } 
  return UPLL_RC_SUCCESS;
}

void MoMgrImpl::DumpRenameInfo(ConfigKeyVal *ikey) {
  UPLL_FUNC_TRACE;
  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_TRACE("Input key is NULL");
    return;
  }
  key_rename_vnode_info *rename = reinterpret_cast<key_rename_vnode_info *>(
      ikey->get_key());
  UPLL_LOG_TRACE("The Rename for the Key type %d", ikey->get_key_type());
  if (strlen(reinterpret_cast<char *>(rename->new_unc_vtn_name)))
    UPLL_LOG_TRACE("new_unc_vtn_name = %s",
                   reinterpret_cast<char*>(rename->new_unc_vtn_name));
  if (strlen(reinterpret_cast<char *>(rename->old_unc_vtn_name)))
    UPLL_LOG_TRACE("old_unc_vtn_name = %s",

                   reinterpret_cast<char*>(rename->old_unc_vtn_name));
  if (strlen(reinterpret_cast<char *>(rename->new_unc_vnode_name)))
    UPLL_LOG_TRACE("new_unc_vnode_name = %s",
                   reinterpret_cast<char*>(rename->new_unc_vnode_name));
  if (strlen(reinterpret_cast<char *>(rename->old_unc_vnode_name)))
    UPLL_LOG_TRACE("old_unc_vnode_name = %s",
                   reinterpret_cast<char*>(rename->old_unc_vnode_name));
  if (strlen(reinterpret_cast<char *>(rename->ctrlr_vtn_name)))
    UPLL_LOG_TRACE("ctrlr_vtn_name = %s",
                   reinterpret_cast<char*>(rename->ctrlr_vtn_name));
  if (strlen(reinterpret_cast<char *>(rename->ctrlr_vnode_name)))
    UPLL_LOG_TRACE("ctrlr_vnode_name = %s",
                   reinterpret_cast<char*>(rename->ctrlr_vnode_name));
}



upll_rc_t MoMgrImpl::RenameMo(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi,
                              const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // char ctrlr_name[10]= "ctrlr1"; //its input argument.
  ConfigKeyVal *okey = NULL;
  ConfigKeyVal *rename_info = NULL;
  ConfigKeyVal *dup_ikey = NULL;
  // char unc_old_name[32];
  bool renamed   = false;
  bool no_rename = false;
  UPLL_LOG_TRACE("RenameMo");
  if (NULL == ikey || NULL == req) {
    UPLL_LOG_DEBUG(" Given Input is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage Return Failure = %d ", result_code);
    return result_code;
  }
  // The new name copy to the okey for checking DT_CANDIDATE
  result_code = SwapKeyVal(ikey, okey, dmi,
                 reinterpret_cast<uint8_t *>(const_cast<char *>(ctrlr_id)),
                 no_rename);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("SwapKeyVal Return Failure = %d ", result_code);
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = GetChildConfigKey(dup_ikey, ikey);
  UPLL_LOG_TRACE("DupRenameKey %s", (dup_ikey->ToStrAll()).c_str());
  UPLL_LOG_TRACE(" Rename No Rename falg = %d", no_rename);
  UPLL_LOG_TRACE(" After SwapKeyVal Ikey %s", (dup_ikey->ToStrAll()).c_str());
  UPLL_LOG_TRACE(" After SwapKeyVal Okey %s", (okey->ToStrAll()).c_str());
  /* the vnode name should not be same */
  // Set The controller ID into the dup_ikey & okey
  SET_USER_DATA_CTRLR(dup_ikey, ctrlr_id);
  SET_USER_DATA_CTRLR(okey, ctrlr_id);
  if (!no_rename) {
    switch (dup_ikey->get_key_type()) {
    case UNC_KT_VBRIDGE:
    case UNC_KT_VROUTER:
    case UNC_KT_VTERMINAL: {
      /**/
      ConfigKeyVal* vnode_ckv = NULL;
      result_code = GetChildConfigKey(vnode_ckv, ikey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed");
        DELETE_IF_NOT_NULL(okey);
        DELETE_IF_NOT_NULL(dup_ikey);
        return UPLL_RC_ERR_GENERIC;
      }
      DbSubOp vnode_dbop = { kOpReadSingle, kOpMatchNone,
                             kOpInOutCtrlr | kOpInOutDomain };
      result_code = ReadConfigDB(vnode_ckv, req->datatype, UNC_OP_READ,
                                 vnode_dbop, dmi);
      if (UPLL_RC_SUCCESS != result_code &&
          UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("Read configDb failed %d", result_code);
        DELETE_IF_NOT_NULL(okey);
        DELETE_IF_NOT_NULL(dup_ikey);
        DELETE_IF_NOT_NULL(vnode_ckv);
        return result_code;
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        result_code = GetRenamedUncKey(vnode_ckv, req->datatype, dmi,
                                       (uint8_t *)(ctrlr_id));
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("Read configDb failed %d", result_code);
          DELETE_IF_NOT_NULL(okey);
          DELETE_IF_NOT_NULL(dup_ikey);
          DELETE_IF_NOT_NULL(vnode_ckv);
          return result_code;
        }
      }
      controller_domain ctrlr_dom;
      memset(&ctrlr_dom, 0, sizeof(controller_domain));
      GET_USER_DATA_CTRLR_DOMAIN(vnode_ckv, ctrlr_dom)
      SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
      DELETE_IF_NOT_NULL(vnode_ckv);

      result_code =  VnodeChecks(okey, req->datatype, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        DELETE_IF_NOT_NULL(dup_ikey);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      result_code = PartialImport_VnodeChecks(okey, UPLL_DT_CANDIDATE,
                                              ctrlr_id, dmi);
      if (UPLL_RC_SUCCESS != result_code) {
        DELETE_IF_NOT_NULL(dup_ikey);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
    }
     break;
    case UNC_KT_VLINK: {
      DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutDomain} ;
      result_code = ReadConfigDB(okey, UPLL_DT_CANDIDATE, UNC_OP_READ, dbop,
                                 dmi, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code &&
          UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        DELETE_IF_NOT_NULL(dup_ikey);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      if (UPLL_RC_SUCCESS == result_code) {
        uint8_t *ca_ctrlr_id = NULL;
        GET_USER_DATA_CTRLR(okey, ca_ctrlr_id);
        if (!strcmp((const char*)ca_ctrlr_id, ctrlr_id)) {
          // no issue continue
          // same controller match
          UPLL_LOG_TRACE("Same name exists on same controller");
        } else {
          UPLL_LOG_TRACE("Same name exists on different controller");
          delete dup_ikey;
          delete okey;
          return UPLL_RC_ERR_INSTANCE_EXISTS;
        }
      }
      okey->SetCfgVal(NULL);
      result_code = ReadConfigDB(okey, req->datatype, UNC_OP_READ, dbop,
                                 dmi, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code &&
          UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        DELETE_IF_NOT_NULL(dup_ikey);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      if (UPLL_RC_SUCCESS == result_code) {
        UPLL_LOG_TRACE("Same Name already exists in Import configuraiton");
        delete dup_ikey;
        delete okey;
        return UPLL_RC_ERR_INSTANCE_EXISTS;
      }
    }
    default:
       break;
    }
  }
  // Checks if the PFC name is already renamed
  dup_ikey->SetCfgVal(NULL);
  result_code = GetRenamedUncKey(dup_ikey, req->datatype, dmi,
                  reinterpret_cast<uint8_t *>(const_cast<char *>(ctrlr_id)));
  if (result_code == UPLL_RC_SUCCESS) {
     UPLL_LOG_TRACE("Given PFC Exists in Import DB %s", (dup_ikey->ToStrAll()).c_str());
     result_code = UpdateConfigDB(dup_ikey, req->datatype, UNC_OP_READ,
                                   dmi, MAINTBL);
     /* Entry in rename table but no corresponding entry in main table */
     if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
       UPLL_LOG_ERROR("UpdateConfigDB Return Failure = %d ", result_code);
       DELETE_IF_NOT_NULL(dup_ikey);
       DELETE_IF_NOT_NULL(okey);
       return UPLL_RC_ERR_GENERIC;
     }
     renamed = true;
    } else if (result_code != UPLL_RC_SUCCESS &&
               result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_DEBUG("GetRenamedUncKey failed. Result : %d", result_code);
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(dup_ikey);
      return UPLL_RC_ERR_GENERIC;
    }
    /* ensure no other vnode has the renamed name in the main table */
    if (UNC_KT_VTN != okey->get_key_type()) {
      result_code = UpdateConfigDB(okey, req->datatype,
				     UNC_OP_READ, dmi, MAINTBL);
      if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code &&
	    UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
	   UPLL_LOG_DEBUG("UpdateConfigDB Failed %d", result_code);
	    DELETE_IF_NOT_NULL(okey);
	    DELETE_IF_NOT_NULL(dup_ikey);
	   return result_code;
      }
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
	  UPLL_LOG_DEBUG("Record Exists in DT IMPORT = %d ", result_code);
	   DELETE_IF_NOT_NULL(okey);
	   DELETE_IF_NOT_NULL(dup_ikey);
	  return result_code;
      }
    }
    /* Ensure the renamed name does not correspond to an existing UNC name */
    if (!no_rename) {
      if (UNC_KT_VTN != okey->get_key_type()) {
        result_code = UpdateConfigDB(okey, UPLL_DT_CANDIDATE, UNC_OP_READ,
                                     dmi, MAINTBL);
        if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
          UPLL_LOG_DEBUG("UpdateConfigDB Failed %d", result_code);
          DELETE_IF_NOT_NULL(okey);
          DELETE_IF_NOT_NULL(dup_ikey);
          return result_code;
        }
        if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
          UPLL_LOG_DEBUG("Record Exists in DT CANDIDATE %d ", result_code);
          DELETE_IF_NOT_NULL(okey);
          DELETE_IF_NOT_NULL(dup_ikey);
          return result_code;
        }
      }
    }
    if (renamed)    {
        UPLL_LOG_TRACE("Before Read from Rename Table %s", (dup_ikey->ToStrAll()).c_str());
        dup_ikey->SetCfgVal(NULL);
        DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr, kOpInOutCtrlr | kOpInOutDomain };
        result_code = ReadConfigDB(dup_ikey, req->datatype, UNC_OP_READ, dbop,
                                   dmi, RENAMETBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_TRACE("ERROR :: %d:%s:%s", __LINE__, __FILE__, __func__);
          DELETE_IF_NOT_NULL(okey);
          DELETE_IF_NOT_NULL(dup_ikey);
          return result_code;
        }
    }
    /* Get The PFC, Old UNC, and New UNC name and maintains in rename_info
     * dup_ikey -> Old Unc Name (or) Controller Name
     * okey -> New Name
     * rename_info -> Empty
     * */
    SET_USER_DATA_FLAGS(okey, no_rename);
    result_code = GetRenameInfo(dup_ikey, okey, rename_info, dmi,
                                ctrlr_id, renamed);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetRenameInfo is Failed %d", result_code);
      DELETE_IF_NOT_NULL(dup_ikey);
      DELETE_IF_NOT_NULL(okey);
      DELETE_IF_NOT_NULL(rename_info);
      return result_code;
    }
    UPLL_LOG_TRACE("The Control Id is %s", ctrlr_id);
    /* Update the New name into the tabels */
    UPLL_LOG_TRACE("Rename Info detail");
    DumpRenameInfo(rename_info);
    result_code =  UpdateTables(req, rename_info, renamed, dmi, no_rename);
    UPLL_LOG_TRACE("UpdateTable Result is %d", result_code);
    DELETE_IF_NOT_NULL(okey);
    DELETE_IF_NOT_NULL(dup_ikey);
    DELETE_IF_NOT_NULL(rename_info);
    return result_code;
}

upll_rc_t MoMgrImpl::MergeImportToCandidate(unc_key_type_t keytype,
                                            const char *ctrlr_name,
                                            DalDmlIntf *dmi,
                                            upll_import_type import_type) {
    UPLL_FUNC_TRACE;
    if (OVERLAY_KT(keytype)|| UNKNOWN_KT(keytype))
      return UPLL_RC_SUCCESS;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    UPLL_LOG_TRACE("MergeImportToCandidate ");
    unc_keytype_operation_t op[] = {
      UNC_OP_CREATE,
      UNC_OP_UPDATE };
    for ( unsigned int operation = 0; operation < (sizeof(op) / sizeof(op[0]));
         operation++) {
      if (UPLL_IMPORT_TYPE_FULL == import_type &&
          UNC_OP_UPDATE == op[operation]) {
        return UPLL_RC_SUCCESS;
      }
      for (int tbl = MAINTBL; tbl < ntable; tbl++) {
        uudst::kDalTableIndex tbl_index;
        tbl_index = GetTable((MoMgrTables)tbl, UPLL_DT_IMPORT);
        /* skipping VTN Main Table */
        if (tbl_index < uudst::kDalNumTables && tbl_index != 0) {
          UPLL_LOG_TRACE("Merging the Keytype %d, Table %d, Table index %d ",
                         keytype, tbl, tbl_index);
          if (table[tbl]->get_key_type() != keytype)
            return UPLL_RC_ERR_GENERIC;
          if (UNC_OP_UPDATE == op[operation]) {
            // Only for Update Operation to avoid CS and ST
            // val binding
            DalBindInfo dal_bind_info(tbl_index);
            result_code = BindKeyAndValForMerge(&dal_bind_info,
                                        UPLL_DT_CANDIDATE,(MoMgrTables)tbl,
                                        tbl_index);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_DEBUG("Error while binding %d", result_code);
              return result_code;
            }
            result_code = DalToUpllResCode(dmi->CopyModifiedRecords(
                    UPLL_DT_CANDIDATE,
                    UPLL_DT_IMPORT, tbl_index,
                    &dal_bind_info, op[operation]));

          } else {
            // Only for Create operation.
            result_code = DalToUpllResCode(dmi->CopyModifiedRecords(
                    UPLL_DT_CANDIDATE,
                    UPLL_DT_IMPORT, tbl_index,
                    NULL,op[operation]));
          }
          UPLL_LOG_TRACE("%d Table is completed", tbl);
          if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)
            result_code = UPLL_RC_SUCCESS;
        }
        if (UPLL_RC_SUCCESS != result_code) {
          break;
        }
      }
      if (UPLL_RC_SUCCESS != result_code)
        break;
    }
// convert dal_result to result_code
return result_code;
}

upll_rc_t MoMgrImpl::ImportClear(unc_key_type_t keytype,
                                 const char *ctrlr_id,
                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  auto_rename_.clear();
  return (ClearConfiguration(keytype, dmi, UPLL_DT_IMPORT));
}

upll_rc_t MoMgrImpl::GetDiffRecord(ConfigKeyVal *ckv_running,
                                   ConfigKeyVal *ckv_audit,
                                   uuc::UpdateCtrlrPhase phase, MoMgrTables tbl,
                                   ConfigKeyVal *&okey,
                                   DalDmlIntf *dmi,
                                   bool &invalid_attr,
                                   bool check_audit_phase) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv_dup = NULL;
  okey = NULL;
  upll_keytype_datatype_t dt_type = (phase == uuc::kUpllUcpDelete)?
      UPLL_DT_AUDIT : UPLL_DT_RUNNING;
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutDomain|
    kOpInOutFlag | kOpInOutCs};
  switch (phase) {
    case uuc::kUpllUcpCreate:
    case uuc::kUpllUcpDelete:
      if (tbl == CTRLRTBL) {
        UPLL_LOG_TRACE("Created/Deleted record for ctrlr_tbl is %s ", ckv_running->ToStrAll().c_str());
        dbop.inoutop = kOpInOutCtrlr | kOpInOutDomain;
        result_code = GetChildConfigKey(okey, ckv_running);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed. err_code & phase %d %d",
                         result_code, phase);
          return result_code;
        }
        result_code = ReadConfigDB(okey, dt_type,
                                   UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadConfigDB failed. err_code & phase %d %d",
                         result_code, phase);
          return result_code;
        }
      } else {
        UPLL_LOG_TRACE("Created/Deleted  record is %s ", ckv_running->ToStrAll().c_str());
        result_code = DupConfigKeyVal(okey, ckv_running, tbl);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("DupConfigKeyVal failed. err_code & phase %d %d",
                         result_code, phase);
          return result_code;
        }
      }
      break;
    case uuc::kUpllUcpUpdate:
      if (tbl == CTRLRTBL) {
        UPLL_LOG_TRACE("UpdateRecord  record for Ctrlr_tbl is %s ", ckv_running->ToStrAll().c_str());
        UPLL_LOG_TRACE("UpdateRecord  record for Ctrlr_tbl is %s ", ckv_audit->ToStrAll().c_str());
        result_code = GetChildConfigKey(okey, ckv_running);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("GetChildConfigKey for running record failed. "
                         "err_code & phase %d %d", result_code, phase);
          return result_code;
        }
        result_code = ReadConfigDB(okey, UPLL_DT_RUNNING,
                                   UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadConfigDB from running failed. "
                         "err_code & phase %d %d", result_code, phase);
          return result_code;
        }
        result_code = GetChildConfigKey(ckv_dup, ckv_audit);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("GetChildConfigKey for audit record failed. "
                         "err_code & phase %d %d", result_code, phase);
          DELETE_IF_NOT_NULL(ckv_dup);
          return result_code;
        }
        result_code = ReadConfigDB(ckv_dup, UPLL_DT_AUDIT,
                                   UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadConfigDB from audit failed. "
                         "err_code & phase %d %d", result_code, phase);
          DELETE_IF_NOT_NULL(ckv_dup);
          return result_code;
        }
      } else {
        UPLL_LOG_TRACE("UpdateRecord  record  is %s ", ckv_running->ToStrAll().c_str());
        UPLL_LOG_TRACE("UpdateRecord  record  is %s ", ckv_audit->ToStrAll().c_str());
        result_code = DupConfigKeyVal(okey, ckv_running, tbl);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("DupConfigKeyVal failed for running record. "
                         "err_code & phase %d %d", result_code, phase);
          return result_code;
        }
        result_code = DupConfigKeyVal(ckv_dup, ckv_audit, tbl);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("DupConfigKeyVal failed for audit record. "
                         "err_code & phase %d %d", result_code, phase);
          DELETE_IF_NOT_NULL(ckv_dup);
          return result_code;
        }
      }
      if (GetVal(okey) != NULL &&
          GetVal(ckv_dup) != NULL) {
        void *val1 = GetVal(okey);
        invalid_attr = FilterAttributes(val1, GetVal(ckv_dup), false,
                                        UNC_OP_UPDATE);
      }
      if (check_audit_phase) {
        if ((okey != NULL) && (ckv_dup != NULL)) {
          ConfigVal *next_val = (ckv_dup->get_cfg_val())->DupVal();
          uint8_t rename_flag = 0;
          GET_USER_DATA_FLAGS(ckv_dup->get_cfg_val(), rename_flag)
          SET_USER_DATA_FLAGS(next_val, rename_flag)
          okey->AppendCfgVal(next_val);
        }
      }
      break;
    default:
      UPLL_LOG_INFO("Invalid operation %d", phase);
      return UPLL_RC_ERR_BAD_REQUEST;
      break;
  }
  DELETE_IF_NOT_NULL(ckv_dup);
  return result_code;
}

upll_rc_t MoMgrImpl::PerformRedirectTranslationForAudit(
    ConfigKeyVal *ikey,
    DalDmlIntf *dmi,
    upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_INFO("Input key is null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (!ikey->get_cfg_val() || !ikey->get_cfg_val()->get_val()) {
    UPLL_LOG_DEBUG(" val is null ");
    return UPLL_RC_SUCCESS;
  }
  val_flowfilter_entry_t *val_flowfilter_entry =
      static_cast<val_flowfilter_entry_t *>(
          ikey->get_cfg_val()->get_val());
  if ((val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
       UNC_VF_VALID) &&
      (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
       UNC_VF_VALID ) &&
      (val_flowfilter_entry->redirect_direction
       == UPLL_FLOWFILTER_DIR_OUT )) {
    // continue to translate;
  } else {
    return UPLL_RC_SUCCESS;
  }

  MoMgrImpl *vbrif_mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
  if (NULL == vbrif_mgr) {
    UPLL_LOG_DEBUG("Unable to get vBridge Interface momgr");
    return UPLL_RC_ERR_GENERIC;
  }

  ConfigKeyVal *okey = NULL;
  result_code = vbrif_mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("GetChildConfigKey failed for vBridge key struct - %d",
                  result_code);
    return result_code;
  }

  key_vbr_if_t *vbrif_key = static_cast<key_vbr_if_t*>(
      okey->get_key());
  switch (ikey->get_key_type()) {
    case UNC_KT_VBR_FLOWFILTER_ENTRY:
      uuu::upll_strncpy(vbrif_key->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vbr_flowfilter_entry_t *>
                        (ikey->get_key())->flowfilter_key.vbr_key.vtn_key.vtn_name,
                        kMaxLenVtnName + 1);
      break;
    case UNC_KT_VBRIF_FLOWFILTER_ENTRY:
      uuu::upll_strncpy(vbrif_key->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vbr_if_flowfilter_entry_t *>
                        (ikey->get_key())->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
                        kMaxLenVtnName + 1);
      break;
    case UNC_KT_VRTIF_FLOWFILTER_ENTRY:
      uuu::upll_strncpy(vbrif_key->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
                        (ikey->get_key())->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
                        kMaxLenVtnName + 1);
      break;

    case UNC_KT_VTERMIF_FLOWFILTER_ENTRY:
      uuu::upll_strncpy(vbrif_key->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
                        (ikey->get_key())->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
                        kMaxLenVtnName + 1);
      break;
    default:
      UPLL_LOG_INFO("Error: bad key type %d", ikey->get_key_type());
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_SUCCESS;
  }

  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
  GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
  SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);

  uuu::upll_strncpy(vbrif_key->vbr_key.vbridge_name,
                    val_flowfilter_entry->redirect_node,
                    (kMaxLenVnodeName + 1));
  uuu::upll_strncpy(vbrif_key->if_name,
                    val_flowfilter_entry->redirect_port,
                    (kMaxLenInterfaceName + 1));
  DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain, kOpInOutFlag};
  result_code = vbrif_mgr->ReadConfigDB(okey, dt_type,
                                        UNC_OP_READ , dbop , dmi, MAINTBL);
  if (UPLL_RC_SUCCESS == result_code) {
    val_drv_vbr_if *val_drv_vbr =
        reinterpret_cast<val_drv_vbr_if *>(GetVal(okey));
    if (val_drv_vbr->vbr_if_val.valid[UPLL_IDX_PM_VBRI] == UNC_VF_VALID) {
      UPLL_LOG_DEBUG("redirect node/port has port map "
                     " and redirect-direction is OUT");
      uuu::upll_strncpy(val_flowfilter_entry->redirect_node,
                        val_drv_vbr->vex_name,
                        (kMaxLenInterfaceName + 1));
      uuu::upll_strncpy(val_flowfilter_entry->redirect_port,
                        val_drv_vbr->vex_if_name,
                        (kMaxLenInterfaceName + 1));
      DELETE_IF_NOT_NULL(okey);
      return UPLL_RC_SUCCESS;
    }
  } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    // Nothing to do
  } else {
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  DELETE_IF_NOT_NULL(okey);
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::AuditUpdateController(unc_key_type_t keytype,
                                           const char *ctrlr_id,
                                           uint32_t session_id,
                                           uint32_t config_id,
                                           uuc::UpdateCtrlrPhase phase,
                                           DalDmlIntf *dmi,
                                           ConfigKeyVal **err_ckv,
                                           KTxCtrlrAffectedState *ctrlr_affected) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result = uud::kDalRcSuccess;
  MoMgrTables tbl  = MAINTBL;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  ConfigKeyVal  *ckv_running = NULL;
  ConfigKeyVal  *ckv_audit = NULL;
  ConfigKeyVal  *ckv_drvr = NULL;
  ConfigKeyVal  *resp = NULL;
  DalCursor *cursor = NULL;
  bool invalid_attr = false;
  // Specifies true for audit update transaction
  // else false default
  const bool audit_update_phase = true;
  uint8_t flags = 0;
  uint8_t *in_ctrlr = reinterpret_cast<uint8_t *>(const_cast<char *>(ctrlr_id));
  // Decides whether to retrieve from controller table or main table
  bool pom_update_kt = false;
  POM_UPDATE_KTS(keytype, pom_update_kt)
  if (pom_update_kt) {
    return GlobalAuditUpdateController(keytype, ctrlr_id, session_id, config_id,
                                       phase, dmi, err_ckv, ctrlr_affected);
  }
  GET_TABLE_TYPE(keytype, tbl);
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
      ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
       ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));
  if (phase == uuc::kUpllUcpDelete2)
    return result_code;
  if (op == UNC_OP_INVALID) {
    UPLL_LOG_INFO("Invalid operation received-%d", op);
    // Not a valid operation
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  /* retreives the delta of running and audit configuration */
  UPLL_LOG_TRACE("Operation is %d", op);
  // Update operation not supported for UNC_KT_VTN
  if ((op == UNC_OP_UPDATE) && (keytype == UNC_KT_VTN))
    return UPLL_RC_SUCCESS;
  bool auditdiff_with_flag = false;
  if ((op == UNC_OP_CREATE) || (op == UNC_OP_DELETE))
    auditdiff_with_flag = true;
    
  // Get CREATE, DELETE and UPDATE object information based on the table 'tbl'
  // between running configuration and audit configuration where 'ckv_running' parameter
  // contains the running information and
  // the 'ckv_audit' parameter contains the audit configuration.
  result_code = DiffConfigDB(UPLL_DT_RUNNING, UPLL_DT_AUDIT, op,
                             ckv_running, ckv_audit,
                             &cursor, dmi, in_ctrlr, tbl, true, auditdiff_with_flag);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DiffConfigDB failed - %d", result_code);
    return result_code;
  }
  // Iterate loop to get next record
  while (uud::kDalRcSuccess == (db_result = dmi->GetNextRecord(cursor))) {
    UPLL_LOG_TRACE("Diff Running Record for Keytype: Operation:  is %d %d\n %s",
                   keytype, op, ckv_running->ToStrAll().c_str());
    /* ignore records of another controller for create and update operation */
    if (phase != uuc::kUpllUcpDelete) {
      uint8_t *db_ctrlr = NULL;
      GET_USER_DATA_CTRLR(ckv_running, db_ctrlr);
      if ((!db_ctrlr) || (db_ctrlr && strncmp(reinterpret_cast<const char *>(db_ctrlr),
          reinterpret_cast<const char *>(ctrlr_id),
          strlen(reinterpret_cast<const char *>(ctrlr_id)) + 1))) {
        continue;
      }
    }
    if (op == UNC_OP_UPDATE)
      UPLL_LOG_TRACE("Diff Audit Record for Keytype: Operation:  is %d %d\n %s",
                     keytype, op, ckv_audit->ToStrAll().c_str());
    // To fetch the records from the running which differ with the audit records.
    // If audit_update_phase flag is true and operation is UNC_OP_UPDATE
    // append the audit ConfigVal into running ConfigKeyVal
    // 'ckv_drvr' contains both running and audit ConfigVal
    result_code =  GetDiffRecord(ckv_running, ckv_audit, phase, tbl,
                                 ckv_drvr, dmi, invalid_attr, audit_update_phase);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("GetDiffRecord failed err code is %d", result_code);
      DELETE_IF_NOT_NULL(ckv_running);
      DELETE_IF_NOT_NULL(ckv_audit);
      DELETE_IF_NOT_NULL(ckv_drvr);
      dmi->CloseCursor(cursor, true);
      return result_code;
    }
    if (invalid_attr) {
      DELETE_IF_NOT_NULL(ckv_drvr);
      // Assuming that the diff found only in ConfigStatus
      // Setting the value as OnlyCSDiff in the out parameter ctrlr_affected
      // The value Configdiff should be given more priority than the value
      // only cs.
      // So, if the out parameter ctrlr_affected has already value as configdiff
      // then dont change the value
      if (*ctrlr_affected != uuc::kCtrlrAffectedConfigDiff) {
        UPLL_LOG_INFO("Setting the ctrlr_affected to OnlyCSDiff for KT %u",
                      keytype);
        *ctrlr_affected = uuc::kCtrlrAffectedOnlyCSDiff;
      }
      continue;
    }
    // Case1: commit(del) - Check in AUDIT since no info exists in RUNNING
    // Case2: Commit(Cr/upd) - Check in RUNNING always
    upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
        UPLL_DT_AUDIT : UPLL_DT_RUNNING;
    // not_send_to_drv is to decide whether the configuration needs
    // to be sent to controller or not
    bool not_send_to_drv = false;
    // Perform semantic validation and corresponding
    // vexternal/vexternalif conversion on key type specific,
    // before sending to driver.
    result_code = AdaptValToDriver(ckv_drvr, NULL, op,
                                   dt_type, keytype, dmi, not_send_to_drv, audit_update_phase);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_TRACE("AdaptValToDriver failed %d", result_code);
      DELETE_IF_NOT_NULL(ckv_running);
      DELETE_IF_NOT_NULL(ckv_audit);
      DELETE_IF_NOT_NULL(ckv_drvr);
      dmi->CloseCursor(cursor, true);
      return result_code;
    }
    if (not_send_to_drv) {
      UPLL_LOG_TRACE("%s Configuration is not sent to controller",
                     (ckv_drvr->ToStr()).c_str());
      DELETE_IF_NOT_NULL(ckv_drvr);
      continue;
    }
    // Get controller & domain-id from ConfigKeyVal 'ckv_drvr'
    GET_USER_DATA_CTRLR_DOMAIN(ckv_drvr, ctrlr_dom);
    if (!OVERLAY_KT(keytype)) {
      // Duplicates the running ConfigKeyVal 'ckv_drvr' into 'resp'
      // 'resp' is used for vlanMapMgr and in AdaptValToVtnService().
      result_code = DupConfigKeyVal(resp, ckv_drvr, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" DupConfigKeyVal failed err code(%d)",
                       result_code);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
      // resp will have State structure,next cfg val, which is not required
      // for further processing, since during update phase old val struct
      // will come as next cfg val, it might cause issue while
      // accessing that memory
      //
      if (resp->get_cfg_val() != NULL) {
          resp->get_cfg_val()->DeleteNextCfgVal();
      }

      // GET_USER_DATA_FLAGS determines whether the vLanMap is a user
      // configured vLanMap or a boundary vLanMap
      GET_USER_DATA_FLAGS(ckv_drvr, flags);
      // Get the controller key for the renamed unc key.
      result_code = GetRenamedControllerKey(ckv_drvr, dt_type,
                                            dmi, &ctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" GetRenamedControllerKey failed err code(%d)",
                       result_code);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(resp);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
    }

    // Returns when controller-id and domain-id is not present.
    if (ctrlr_dom.ctrlr != NULL) {
      bool domain = false;
      KEYTYPE_WITHOUT_DOMAIN(keytype, domain);
      if (!domain) {
        if (NULL == ctrlr_dom.domain) {
          UPLL_LOG_INFO(" domain is NULL");
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(resp);
          dmi->CloseCursor(cursor, true);
          return UPLL_RC_ERR_GENERIC;
        }
      }
    } else {
      UPLL_LOG_DEBUG("Controller Id is NULL");
      DELETE_IF_NOT_NULL(ckv_running);
      DELETE_IF_NOT_NULL(ckv_audit);
      DELETE_IF_NOT_NULL(ckv_drvr);
      DELETE_IF_NOT_NULL(resp);
      dmi->CloseCursor(cursor, true);
      return UPLL_RC_ERR_GENERIC;
    }

    UPLL_LOG_TRACE("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                   ctrlr_dom.domain);
    
    IpcResponse ipc_response;
    memset(&ipc_response, 0, sizeof(IpcResponse));
    IpcRequest ipc_req;
    memset(&ipc_req, 0, sizeof(IpcRequest));
    ipc_req.header.clnt_sess_id = session_id;
    ipc_req.header.config_id = config_id;
    ipc_req.header.operation = op;
    ipc_req.header.datatype = UPLL_DT_CANDIDATE;
    ipc_req.ckv_data = ckv_drvr;
    // To populate IPC request.
    if (!IpcUtil::SendReqToDriver((const char *)ctrlr_dom.ctrlr, reinterpret_cast<char *>
                                  (ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
                                  PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_response)) {
      UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
                    ckv_drvr->get_key_type(), reinterpret_cast<char *>(ctrlr_dom.ctrlr));
      DELETE_IF_NOT_NULL(resp);
      DELETE_IF_NOT_NULL(ckv_running);
      DELETE_IF_NOT_NULL(ckv_audit);
      DELETE_IF_NOT_NULL(ckv_drvr);
      DELETE_IF_NOT_NULL(ipc_response.ckv_data);
      dmi->CloseCursor(cursor, true);
      return ipc_response.header.result_code;
    }
    /* VlanmapOnBoundary: Added vlanmap check */
    if (ckv_running->get_key_type() == UNC_KT_VBR_VLANMAP &&
        ipc_response.header.result_code != UPLL_RC_SUCCESS) {
      SET_USER_DATA_FLAGS(ckv_drvr, flags);
      UPLL_LOG_TRACE("Flags = %u", flags);
      UPLL_LOG_DEBUG("IpcSend failed %d", result_code);
      VlanMapMoMgr *vlanmap_mgr = reinterpret_cast<VlanMapMoMgr *>(
          const_cast<MoManager *>(GetMoManager(ckv_running->get_key_type())));
      if (!vlanmap_mgr) {
        UPLL_LOG_DEBUG("Invalid mgr param");
        DELETE_IF_NOT_NULL(resp);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        dmi->CloseCursor(cursor, true);
        return UPLL_RC_ERR_GENERIC;
      }

      /* resp is expected NULL inside TranslateError
       * Inside TranslateError, DupConfigKeyVal is called */
      DELETE_IF_NOT_NULL(resp);
      result_code  = vlanmap_mgr->TranslateError(&resp, ckv_running,
                                                 dmi, dt_type);
      if (result_code != UPLL_RC_SUCCESS) {
        DELETE_IF_NOT_NULL(resp);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
    }
    if (ipc_response.ckv_data) {
      if (ckv_running->get_key_type() == UNC_KT_VBR_IF &&
          ipc_response.header.result_code != UPLL_RC_SUCCESS) {
        VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>
            (const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
        vn_if_type iftype;
        ConfigKeyVal *ck_vlink = NULL;
        if (!mgr) {
          UPLL_LOG_DEBUG("Invalid mgr");
          DELETE_IF_NOT_NULL(resp);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          dmi->CloseCursor(cursor, true);
          return UPLL_RC_ERR_GENERIC;
        }
        ConfigKeyVal *ck_vif = NULL;
        result_code = GetChildConfigKey(ck_vif, ckv_running);
        result_code = mgr->CheckIfMemberOfVlink(ck_vif,
                                                UPLL_DT_RUNNING, ck_vlink, dmi, iftype);
        DELETE_IF_NOT_NULL(ck_vif);
        if (result_code == UPLL_RC_SUCCESS) {
          if ((iftype == kVlinkBoundaryNode1) ||
              (iftype == kVlinkBoundaryNode2)) {
            delete ipc_response.ckv_data;
            SET_USER_DATA_CTRLR(ck_vlink, ctrlr_dom.ctrlr);
            *err_ckv = ck_vlink;
            result_code = ipc_response.header.result_code;
            DELETE_IF_NOT_NULL(resp);
            break;
          }
        } else  {
          SET_USER_DATA_CTRLR(ipc_response.ckv_data, ctrlr_dom.ctrlr);
          DELETE_IF_NOT_NULL(ck_vlink);
          UPLL_LOG_DEBUG("Failed to map boundary if to vlink");
        }
      }
    }
    UPLL_LOG_DEBUG("Result code from driver %d", ipc_response.header.result_code);
    if  (ipc_response.header.result_code != UPLL_RC_SUCCESS) {
      // To convert the value structure read from DB to
      // VTNService during READ operations
      result_code = AdaptValToVtnService(resp, ADAPT_ONE);
      if (result_code != UPLL_RC_SUCCESS &&
          result_code != UPLL_RC_ERR_GENERIC) {
        UPLL_LOG_DEBUG("AdaptValToVtnService failed %d\n", result_code);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(resp);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
      if (phase == uuc::kUpllUcpDelete) {
        *err_ckv = resp;
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        return ipc_response.header.result_code; 
      } else {
        UPLL_LOG_DEBUG("driver return failure err_code is %d", ipc_response.header.result_code);
        *err_ckv = resp;
        ConfigKeyVal *ctrlr_key = NULL;
        // To get duplicate ConfigKeyVal from running ConfigKeyVal,
        // either from CTRLTBL or MAINTBL based on the key type.
        if (CTRLRTBL == (GET_TABLE_TYPE(keytype, tbl))) {
          result_code = DupConfigKeyVal(ctrlr_key, ckv_running, CTRLRTBL);
        } else {
          result_code = DupConfigKeyVal(ctrlr_key, ckv_running, MAINTBL);
        }
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("DupConfigKeyVal failed");
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          dmi->CloseCursor(cursor, true);
          return result_code;
        }
        // To update controller ConfigStatus as UNC_CS_INVALID for the error record.
        if (keytype == UNC_KT_VTN) {
          result_code = UpdateCtrlrConfigStatus(UNC_CS_INVALID,
                                                phase, ctrlr_key);
        } else {
          // To update the audit configuration status as UNC_CS_INVALID for the error record.
          result_code = UpdateAuditConfigStatus(UNC_CS_INVALID,
                                                phase, ctrlr_key, dmi);
        }
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO("UpdateAuditConfigStatus failed");
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          dmi->CloseCursor(cursor, true);
          DELETE_IF_NOT_NULL(ctrlr_key);
          return result_code;
        }
        // To update configuration status.
        result_code = UpdateConfigDB(ctrlr_key, UPLL_DT_RUNNING, UNC_OP_UPDATE,
                                     dmi, tbl);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("UpdateConfigDB failed for ipc response ckv err_code %d",
                         result_code);
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          dmi->CloseCursor(cursor, true);
          DELETE_IF_NOT_NULL(ctrlr_key);
          return result_code;
        }
        DELETE_IF_NOT_NULL(ctrlr_key);
        if (CTRLRTBL == (GET_TABLE_TYPE(keytype, tbl))) {
          result_code = SetConsolidatedStatus(resp, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("SetConsolidatedStatus failed for ipc response ckv err_code %d",
                           result_code);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            DELETE_IF_NOT_NULL(ckv_drvr);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
        }
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        return ipc_response.header.result_code; 
      }
    }
    DELETE_IF_NOT_NULL(ckv_drvr);
    DELETE_IF_NOT_NULL(ipc_response.ckv_data);
    //  *ctrlr_affected = true;

    if (*ctrlr_affected == uuc::kCtrlrAffectedOnlyCSDiff) {
      UPLL_LOG_INFO("Reset ctrlr state from OnlyCSDiff to ConfigDiff  KT %u",
                    keytype);
    }
    UPLL_LOG_DEBUG("Setting the ctrlr_affected to ConfigDiff, KT %u", keytype);
    *ctrlr_affected = uuc::kCtrlrAffectedConfigDiff;
    DELETE_IF_NOT_NULL(resp);
  }
  if (cursor)
    dmi->CloseCursor(cursor, true);
  if (uud::kDalRcSuccess != db_result) {
    UPLL_LOG_DEBUG("GetNextRecord from database failed  - %d", db_result);
    result_code =  DalToUpllResCode(db_result);
  }
  DELETE_IF_NOT_NULL(ckv_running);
  DELETE_IF_NOT_NULL(ckv_audit);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
      ? UPLL_RC_SUCCESS : result_code;
  return result_code;
}

upll_rc_t MoMgrImpl::AuditVoteCtrlrStatus(unc_key_type_t keytype,
    CtrlrVoteStatus *vote_status,
    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if ((vote_status == NULL) || (dmi == NULL)) {
    UPLL_LOG_INFO("vote_status or dmi is null");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv_dup  = NULL;
  ConfigKeyVal *ckv_au_dup  = NULL;
  uuc::UpdateCtrlrPhase operation = uuc::kUpllUcpUpdate;
  MoMgrTables tbl = MAINTBL;
  uint32_t ctrlr_result  = vote_status->upll_ctrlr_result;
  uint8_t *ctrlr_id =
    reinterpret_cast<uint8_t *>((char *)(vote_status->ctrlr_id.c_str()));
  UPLL_LOG_DEBUG("KT %d: controller id & vote result  is %s %d",
                 keytype, ctrlr_id, ctrlr_result);
  switch (ctrlr_result) {
    case UPLL_RC_SUCCESS: /* No Operation */
      break;
    case UNC_RC_CTR_DISCONNECTED:
      break;
    default:
      /* retrieves the error configkeyval one by one and if the keytype matches,
         rename the key with UNC name */
      ConfigKeyVal *ckv_drv_rslt  = NULL;
      for (ckv_drv_rslt  = vote_status->err_ckv;
          ckv_drv_rslt != NULL; ckv_drv_rslt = ckv_drv_rslt->get_next_cfg_key_val()) {
        if (ckv_drv_rslt->get_key_type() != keytype)
          continue;
        /* Get the Unc key */

        if (!OVERLAY_KT(keytype)) {
          // Perform RenameUncKey and get logical specific key values from db
          result_code = AdaptErrValToVtn(ckv_drv_rslt, dmi, ctrlr_id,
                                         UPLL_DT_RUNNING);
          if (result_code != UPLL_RC_SUCCESS &&
              result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_INFO("AdaptErrValToVtn failed %d", result_code);
            DELETE_IF_NOT_NULL(ckv_dup);
            return result_code;
          }

          if (ckv_drv_rslt->get_key_type() != keytype) {
            DELETE_IF_NOT_NULL(ckv_dup);
            continue;
          }

          // Convert driver val to logical specific val
          result_code = AdaptValToVtnService(ckv_drv_rslt, ADAPT_ONE);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("AdaptValToVtnService failed %d\n", result_code);
            DELETE_IF_NOT_NULL(ckv_dup);
            return result_code;
         }
        }

        result_code = GetChildConfigKey(ckv_dup, ckv_drv_rslt);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("Duplicate ConfigKeyVal failed  - %d", result_code);
          DELETE_IF_NOT_NULL(ckv_dup);
          return result_code;
        }

        /* Record exist check from running  - if not found, it is a delete*/
        DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCs};
        result_code = ReadConfigDB(ckv_dup, UPLL_DT_RUNNING,
            UNC_OP_READ, dbop, dmi, MAINTBL);
        if (UPLL_RC_SUCCESS == result_code) { /* exists in Running Database */
          /* Record exist check from audit - if exists then update else create*/
          UPLL_LOG_TRACE("Record exist in running tbl result_code %d",
              result_code);
          result_code = GetChildConfigKey(ckv_au_dup, ckv_drv_rslt);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("GetChildConfigKeyVal failed for ckv_au_dup - %d", result_code);
            delete ckv_dup;  //check with Vinoth
            DELETE_IF_NOT_NULL(ckv_au_dup);
            return result_code;
          }
          result_code = ReadConfigDB(ckv_au_dup, UPLL_DT_AUDIT,
              UNC_OP_READ, dbop, dmi, MAINTBL);
          if (UPLL_RC_SUCCESS == result_code) {
            UPLL_LOG_TRACE("Record exist in running & audit tbl result_code %d",
                result_code);
            operation = uuc::kUpllUcpUpdate;
          } else {
            UPLL_LOG_TRACE("Record exist in running tbl only result_code %d",
                result_code);
            operation = uuc::kUpllUcpCreate;
          }
          DELETE_IF_NOT_NULL(ckv_au_dup);
        } else {
          result_code = ReadConfigDB(ckv_dup, UPLL_DT_AUDIT,
              UNC_OP_READ, dbop, dmi, MAINTBL);
          if (UPLL_RC_SUCCESS == result_code) { /* exists in Audit Database */
            UPLL_LOG_TRACE("Record exist in audit tbl only result_code %d",
                result_code);
            operation = uuc::kUpllUcpDelete;
            ckv_dup->DeleteCfgVal();
            UPLL_LOG_DEBUG("ConfigKeyVal failed during AuditVote Phase is %s",
                ckv_dup->ToStrAll().c_str());
          } else {
            UPLL_LOG_TRACE("Record neither exists in running nor "
                "audit tbl result_code %d", result_code);
          }
          DELETE_IF_NOT_NULL(ckv_dup);
          continue;
        }

        if (uuc::kUpllUcpDelete != operation) {
          ConfigKeyVal *temp_ckv_dup = NULL;
          if (CTRLRTBL == (GET_TABLE_TYPE(keytype, tbl))) {
            result_code = GetChildConfigKey(temp_ckv_dup, ckv_drv_rslt);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("Duplicate ConfigKeyVal failed  - %d", result_code);
              DELETE_IF_NOT_NULL(ckv_dup);
              return result_code;
            }
            ConfigKeyVal *ctrlr_ckv = NULL;
            result_code = GetChildConfigKey(ctrlr_ckv, temp_ckv_dup);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("GetChildConfigKeyVal failed %d", result_code);
              DELETE_IF_NOT_NULL(ckv_dup);
              DELETE_IF_NOT_NULL(temp_ckv_dup);
              return result_code;
            }
            SET_USER_DATA_CTRLR(ctrlr_ckv, ctrlr_id);
            DbSubOp dbop1 = {kOpReadMultiple, kOpMatchCtrlr,
              kOpInOutCtrlr | kOpInOutDomain | kOpInOutFlag};
            result_code = ReadConfigDB(ctrlr_ckv, UPLL_DT_RUNNING,
                UNC_OP_READ, dbop1, dmi, CTRLRTBL);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
              DELETE_IF_NOT_NULL(ckv_dup);
              DELETE_IF_NOT_NULL(temp_ckv_dup);
              DELETE_IF_NOT_NULL(ctrlr_ckv);
              return result_code;
            }
            ConfigKeyVal *temp_ctrlr_ckv = ctrlr_ckv;
            while(temp_ctrlr_ckv) {
              if (UNC_KT_VTN == keytype) {
                result_code = UpdateCtrlrConfigStatus(UNC_CS_INVALID,
                    operation, temp_ctrlr_ckv);
              } else {
                result_code = UpdateAuditConfigStatus(UNC_CS_INVALID,
                    operation, temp_ctrlr_ckv, dmi);
              }
              if (UPLL_RC_SUCCESS != result_code) {
                UPLL_LOG_DEBUG("UpdateAuditConfigStatus failed %d", result_code);
                DELETE_IF_NOT_NULL(ckv_dup);
                DELETE_IF_NOT_NULL(temp_ckv_dup);
                DELETE_IF_NOT_NULL(ctrlr_ckv);
                return result_code;
              }
              result_code = UpdateConfigDB(temp_ctrlr_ckv, UPLL_DT_RUNNING,
                  UNC_OP_UPDATE, dmi, CTRLRTBL);
              if (UPLL_RC_SUCCESS != result_code) {
                UPLL_LOG_INFO("UpdateConfigDB failed %d", result_code);
                DELETE_IF_NOT_NULL(ckv_dup);
                DELETE_IF_NOT_NULL(temp_ckv_dup);
                DELETE_IF_NOT_NULL(ctrlr_ckv);
                return result_code;
              }
              temp_ctrlr_ckv = temp_ctrlr_ckv->get_next_cfg_key_val();
            }
            DELETE_IF_NOT_NULL(ctrlr_ckv);
          } else {
            result_code = DupConfigKeyVal(temp_ckv_dup, ckv_dup, MAINTBL);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("DupConfigKeyVal failed %d", result_code);
              DELETE_IF_NOT_NULL(ckv_dup);
              return result_code;
            }
            result_code = UpdateAuditConfigStatus(UNC_CS_INVALID,
                operation, temp_ckv_dup, dmi);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_INFO("UpdateAuditConfigStatus failed %d", result_code);
              DELETE_IF_NOT_NULL(ckv_dup);
              DELETE_IF_NOT_NULL(temp_ckv_dup);
              return result_code;
            }
            /* Configkeyval from driver will not contain
             * flags, controller, domain */
            // Update only config status
            DbSubOp dbop = {kOpReadExist, kOpMatchNone, kOpInOutCs};
            result_code = UpdateConfigDB(temp_ckv_dup, UPLL_DT_RUNNING,
                UNC_OP_UPDATE, dmi, &dbop, MAINTBL);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_INFO("UpdateConfigDB failed %d", result_code);
              DELETE_IF_NOT_NULL(ckv_dup);
              DELETE_IF_NOT_NULL(temp_ckv_dup);
              return result_code;
            }
          }
          result_code = SetConsolidatedStatus(ckv_drv_rslt, dmi);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("SetConsolidatedStatus failed err code %d", result_code);
            DELETE_IF_NOT_NULL(ckv_dup);
            DELETE_IF_NOT_NULL(temp_ckv_dup);
            return result_code;
          }
          DELETE_IF_NOT_NULL(temp_ckv_dup);
        }
        DELETE_IF_NOT_NULL(ckv_dup);

        UPLL_LOG_INFO("Processed err ConfigKeyVal are %s",
                      (ckv_drv_rslt)->ToStrAll().c_str());
      }
  }
  return result_code;
}

upll_rc_t MoMgrImpl::AuditCommitCtrlrStatus(
                                       unc_key_type_t keytype ,
                                       CtrlrCommitStatus *ctrlr_commit_status,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if ((ctrlr_commit_status == NULL) || (dmi == NULL)) {
    UPLL_LOG_DEBUG("ctrlr_commit_status or dmi is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result = uud::kDalRcSuccess;
  ConfigKeyVal *ckv_running = NULL;
  ConfigKeyVal *ckv_audit = NULL;
  ConfigKeyVal *ckv_update = NULL;
  ConfigKeyVal *ctrlr_key = NULL;
  DalCursor *cursor = NULL;
  bool invalid_attr = false;
  uint8_t *ctrlr_id =
         reinterpret_cast<uint8_t *>((char *)(ctrlr_commit_status->ctrlr_id.c_str()));
  uint32_t ctrlr_result = ctrlr_commit_status->upll_ctrlr_result;
  UPLL_LOG_DEBUG("KT %d: controller id & commit result  is %s %d",
                 keytype, ctrlr_id, ctrlr_result);
  MoMgrTables tbl = MAINTBL;
  unc_keytype_operation_t operation = UNC_OP_INVALID;
  GET_TABLE_TYPE(keytype, tbl);
  switch (ctrlr_result) {
  /* if controller returns commit success, set CS Status as applied
     retreived from database */
    case UPLL_RC_SUCCESS:
      /* retreives the delta of running and audit configuration
         - created and updated only */
      for (int loop = uuc::kUpllUcpCreate; loop < uuc::kUpllUcpDelete; ++loop) {
        operation = (loop == uuc::kUpllUcpCreate)?UNC_OP_CREATE:UNC_OP_UPDATE;
/*        if ((operation == UNC_OP_UPDATE) && (keytype == UNC_KT_VTN))
          return UPLL_RC_SUCCESS;*/

         bool auditdiff_with_flag = false;
         auditdiff_with_flag = IS_POM_IF_KT(keytype);
         if (operation == UNC_OP_CREATE || operation == UNC_OP_DELETE)
           auditdiff_with_flag = true; 

        result_code = DiffConfigDB(UPLL_DT_RUNNING, UPLL_DT_AUDIT,
                              operation,
                              ckv_running, ckv_audit,
                              &cursor,
                              dmi, ctrlr_id,  tbl, true, auditdiff_with_flag);
        if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
          UPLL_LOG_DEBUG("No record found for op %d", operation);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          continue;
        }
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("DiffConfigDB failed - %d", result_code);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          // continue;
          return result_code;
        }
        /* Get the record one by one, duplicate the configkeyval
         and update cs status */
        while (uud::kDalRcSuccess == (db_result = dmi->GetNextRecord(cursor))) {
          /* VlanmapOnBoundary: ignore records of another controller
           * for create and update operation */
          uint8_t *db_ctrlr = NULL;
          GET_USER_DATA_CTRLR(ckv_running, db_ctrlr);
          if ((!db_ctrlr) || (db_ctrlr && strncmp(reinterpret_cast<const char *>(db_ctrlr),
                  reinterpret_cast<const char *>(ctrlr_id),
                  strlen(reinterpret_cast<const char *>(ctrlr_id)) + 1))) {
            if (keytype == UNC_KT_VLINK) {
              unc_key_type_t ktype = UNC_KT_ROOT;
              VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>(
                const_cast<MoManager *>(GetMoManager(UNC_KT_VLINK)));
              if (!mgr) {
                UPLL_LOG_DEBUG("Invalid mgr");
                DELETE_IF_NOT_NULL(ckv_running);
                DELETE_IF_NOT_NULL(ckv_audit);
                DELETE_IF_NOT_NULL(ckv_update);
                dmi->CloseCursor(cursor, true);
                return UPLL_RC_ERR_GENERIC;
              }
              ktype = mgr->GetVlinkVnodeIfKeyType(ckv_running, 1);
              if (ktype != UNC_KT_VUNK_IF) {
                GET_USER_DATA_CTRLR(ckv_running->get_cfg_val(), db_ctrlr);
                if (db_ctrlr && strncmp(reinterpret_cast<const char *>
                    (db_ctrlr), reinterpret_cast<const char *>(ctrlr_id),
                    strlen(reinterpret_cast<const char *>(ctrlr_id)) + 1)) {
                  continue;
                }
              } else {
                continue;
              }
            } else {
              continue;
            }
          }
          UPLL_LOG_TRACE("Diff Record: Keytype: Phase:  is %d\n %d\n %s",
                    keytype, loop, ckv_running->ToStrAll().c_str());

          result_code =  GetDiffRecord(ckv_running, ckv_audit,
              (uuc::UpdateCtrlrPhase)loop, tbl,
              ckv_update, dmi, invalid_attr, false);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("GetDiffRecord failed err code is %d", result_code);
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            DELETE_IF_NOT_NULL(ckv_update);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          if (CTRLRTBL == (GET_TABLE_TYPE(keytype, tbl))) {
            result_code = DupConfigKeyVal(ctrlr_key, ckv_running, CTRLRTBL);
          } else {
            result_code = DupConfigKeyVal(ctrlr_key, ckv_running, MAINTBL);
          }
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("DupConfigKeyVal failed");
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            DELETE_IF_NOT_NULL(ckv_update);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          if (UNC_KT_VTN == keytype) {
            result_code = UpdateCtrlrConfigStatus(UNC_CS_APPLIED,
                               (uuc::UpdateCtrlrPhase)loop, ctrlr_key);
          } else {
            result_code = UpdateAuditConfigStatus(UNC_CS_APPLIED,
                              (uuc::UpdateCtrlrPhase)loop, ctrlr_key, dmi);
          }
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("UpdateAuditConfigStatus failed");
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            DELETE_IF_NOT_NULL(ckv_update);
            DELETE_IF_NOT_NULL(ctrlr_key); 
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          result_code = UpdateConfigDB(ctrlr_key, UPLL_DT_RUNNING,
                            UNC_OP_UPDATE, dmi, tbl);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("UpdateConfigDB for config status update failed  %d",
                                 result_code);
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            DELETE_IF_NOT_NULL(ckv_update);
            DELETE_IF_NOT_NULL(ctrlr_key);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          DELETE_IF_NOT_NULL(ctrlr_key);
          if (CTRLRTBL == (GET_TABLE_TYPE(keytype, tbl))) {
            result_code = SetConsolidatedStatus(ckv_update, dmi);
            if (result_code != UPLL_RC_SUCCESS) {
              UPLL_LOG_INFO("SetConsolidatedStatus failed err code %d", result_code);
              DELETE_IF_NOT_NULL(ckv_running);
              DELETE_IF_NOT_NULL(ckv_audit);
              DELETE_IF_NOT_NULL(ckv_update);
              dmi->CloseCursor(cursor, true);
              return result_code;
            }
          }
          DELETE_IF_NOT_NULL(ckv_update);
          if (invalid_attr)
            continue;
        }
        if (cursor) {
          dmi->CloseCursor(cursor, true);
          cursor = NULL;
        }
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
      }
      if (uud::kDalRcSuccess != db_result) {
        UPLL_LOG_DEBUG("GetNextRecord from database failed  - %d",
                          db_result);
        result_code = DalToUpllResCode(db_result);
      }
      result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
                ? UPLL_RC_SUCCESS : result_code;
      break;
    case UNC_RC_CTR_DISCONNECTED:
      break;  // NO OPERATION
    default:
      /* retrieves the error configkeyval one by one and if the keytype matches,
         rename the key with UNC name */
      result_code = AuditVoteCtrlrStatus(keytype, ctrlr_commit_status, dmi);
  }
  return result_code;
}

upll_rc_t MoMgrImpl::AuditEnd(unc_key_type_t keytype,
                              const char *ctrlr_id,
                              DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  return (ClearConfiguration(keytype, dmi, UPLL_DT_AUDIT));
}

// Some configuration types maintain parent-child relationship.
// It is advised to call this API with REVERSE_ORDER
upll_rc_t MoMgrImpl::ClearConfiguration(unc_key_type_t kt,
                                  DalDmlIntf *dmi,
                                  upll_keytype_datatype_t cfg_type) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    DalResultCode db_result;
    for (int tbl = ntable; tbl > MAINTBL; tbl--) {
      uudst::kDalTableIndex tbl_index;
      tbl_index = GetTable((MoMgrTables)(tbl - 1), cfg_type);
      if ((tbl_index >= uudst::kDalNumTables))
        continue;
      // vnode_rename_tbl should be copied only once.
      if (((tbl - 1) == RENAMETBL) && (VNODE_KEYTYPE(kt)) &&
           (kt != UNC_KT_VBRIDGE))
        continue;
      db_result =  dmi->DeleteRecords(cfg_type, tbl_index, NULL, true);
      result_code = DalToUpllResCode(db_result);
      if (result_code != UPLL_RC_SUCCESS &&
          result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)
        return result_code;
    }
    return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::ClearStartup(unc_key_type_t kt,
                                  DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    return(ClearConfiguration(kt, dmi, UPLL_DT_STARTUP));
}

upll_rc_t MoMgrImpl::CopyRunningToStartup(unc_key_type_t kt,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_keytype_datatype_t dest_cfg_type = UPLL_DT_STARTUP;
  upll_keytype_datatype_t src_cfg_type = UPLL_DT_RUNNING;

  for (int tbl = MAINTBL; tbl < ntable; tbl++)  {
    DalResultCode db_result  = uud::kDalRcSuccess;
    const uudst::kDalTableIndex tbl_index = GetTable((MoMgrTables)tbl,
                                                     src_cfg_type);
    if (tbl_index >= uudst::kDalNumTables)
      continue;
    // vnode_rename_tbl should be copied only once.
    if ((tbl == RENAMETBL) && VNODE_KEYTYPE(kt) && (kt != UNC_KT_VBRIDGE))
      continue;
    db_result = dmi->CopyEntireRecords(dest_cfg_type, src_cfg_type,
                                     tbl_index, NULL);
    if ((db_result != uud::kDalRcSuccess) &&
        (db_result != uud::kDalRcRecordNotFound)) {
      return DalToUpllResCode(db_result);
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::CopyEntireConfiguration(
    unc_key_type_t kt,
    DalDmlIntf *dmi,
    upll_keytype_datatype_t dest_cfg_type,
    upll_keytype_datatype_t src_cfg_type) {
  UPLL_FUNC_TRACE;
  for (int tbl = MAINTBL; tbl < ntable; tbl++)  {
    DalResultCode db_result  = uud::kDalRcSuccess;
    const uudst::kDalTableIndex tbl_index = GetTable((MoMgrTables)tbl,
                                                     src_cfg_type);
    if (tbl_index >= uudst::kDalNumTables)
      continue;
    // vnode_rename_tbl should be copied only once.
    if ((tbl == RENAMETBL) && VNODE_KEYTYPE(kt) && (kt != UNC_KT_VBRIDGE))
      continue;
    db_result = dmi->CopyEntireRecords(dest_cfg_type, src_cfg_type,
                                       tbl_index, NULL);
    if (db_result == uud::kDalRcRecordNotFound) {
      UPLL_LOG_TRACE("No records found.");
      db_result = uud::kDalRcSuccess;
    }
    if (db_result != uud::kDalRcSuccess) {
      upll_rc_t err =  DalToUpllResCode(db_result);
      UPLL_LOG_ERROR("Copying for KT %d tbl %d from %d to %d failed. Error=%d",
                     kt, tbl, src_cfg_type, dest_cfg_type, err);
      return err;
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::CopyRunningToCandidate(unc_key_type_t kt,
                                            DalDmlIntf *dmi,
                                            unc_keytype_operation_t op) {
  UPLL_FUNC_TRACE;
  DalResultCode db_result = uud::kDalRcSuccess;
  upll_keytype_datatype_t dest_cfg_type = UPLL_DT_CANDIDATE;
  upll_keytype_datatype_t src_cfg_type = UPLL_DT_RUNNING;
  for (int tbl = MAINTBL; tbl < ntable; tbl++)  {
    const uudst::kDalTableIndex tbl_index = GetTable((MoMgrTables)tbl,
                                                     src_cfg_type);
    if (tbl_index >= uudst::kDalNumTables)
      continue;
    db_result = dmi->CopyModifiedRecords(dest_cfg_type, src_cfg_type, tbl_index,
                  NULL, op);
    if (db_result == uud::kDalRcRecordNotFound) {
      db_result = uud::kDalRcSuccess;
    }
    if (db_result != uud::kDalRcSuccess) {
      break;
    }
  }
  return DalToUpllResCode(db_result);
}

upll_rc_t MoMgrImpl::LoadStartup(unc_key_type_t kt, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // NOTE: Clearing Candidate and Running configuration should be handled by the
  // caller. Here only COPY-INSERT will be performed.

  unc::tclib::TcLibModule *tclib =
      unc::upll::config_momgr::UpllConfigMgr::GetTcLibModule();

  if (tclib == NULL) {
    UPLL_LOG_ERROR("Unable to get tclib module");
    return UPLL_RC_ERR_GENERIC;
  }

  // Check for validity of startup configuration
  // If startup validity is true, Copy startup to running otherwise copy running
  // to candidate.
  UPLL_LOG_TRACE("Startup config validity: %d", tclib->IsStartupConfigValid());
  if (tclib->IsStartupConfigValid() == PFC_TRUE) {
    // copy startup to running
    UPLL_LOG_INFO("Copy KT %d configuration from startup to running", kt);
    // UPLL_LOG_INFO("Initializing Running DB");
    result_code = CopyEntireConfiguration(kt, dmi,
                                          UPLL_DT_RUNNING,
                                          UPLL_DT_STARTUP);
    if (result_code != UPLL_RC_SUCCESS) {
      return result_code;
    }
  }

  UPLL_LOG_INFO("Copy KT %d configuration from running to candidate", kt);
  // copy running to candidate
  UPLL_LOG_INFO("Initializing Candidate DB");
  result_code = CopyEntireConfiguration(kt, dmi,
                                        UPLL_DT_CANDIDATE,
                                        UPLL_DT_RUNNING);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  // TODO(Perf) : As part of performance enhancement, this function can be
  // called only for auto-save. For non auto-save, during copying startup to
  // running, cs status will be initialized
  /* Reinit cs status in RUNNING */
  UPLL_LOG_INFO("Initializing Config Status - KT %d", kt);
  result_code = InitConfigStatus(kt, dmi, UPLL_DT_RUNNING);
  if (result_code != UPLL_RC_SUCCESS) {
    return result_code;
  }

  return result_code;
}

upll_rc_t MoMgrImpl::IsCandidateDirty(unc_key_type_t kt,
                                      bool *dirty,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  DalResultCode db_result = uud::kDalRcSuccess;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  bool identical = false;
  *dirty = false;

  upll_keytype_datatype_t cfg_type1 = UPLL_DT_RUNNING;
  upll_keytype_datatype_t cfg_type2 = UPLL_DT_CANDIDATE;
//  DalBindInfo *matching_attr_info;
  for (int i = MAINTBL; i < ntable; i++) {
    const uudst::kDalTableIndex tbl_index = GetTable((MoMgrTables)i, cfg_type1);
    if (tbl_index >= uudst::kDalNumTables)
      continue;
    if (!dmi->IsTableDirtyShallow(tbl_index)) {
      UPLL_LOG_TRACE("Skipping the tbl %d ", tbl_index);
      continue;
    }
    DalBindInfo dal_bind_info(tbl_index);
    result_code = BindKeyAndVal(&dal_bind_info,
                                cfg_type1, (MoMgrTables)i,
                                tbl_index);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Error while binding %d", result_code);
      return result_code;
    }
    db_result = dmi->CheckRecordsIdentical(cfg_type1,
                                           cfg_type2,
                                           tbl_index,
                                           &dal_bind_info,
                                           &identical);
    result_code = DalToUpllResCode(db_result);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Check Records identical Failed %d", result_code);
      return result_code;
    }
    /* dirty is set if records are identical */
    *dirty = !identical;
    if (!identical) {
      break;
    }
  }
  return result_code;
}

upll_rc_t MoMgrImpl::DalToUpllResCode(DalResultCode result_code) {
  switch (result_code) {
    case uud::kDalRcSuccess:
      return UPLL_RC_SUCCESS;
    case uud::kDalRcConnNotAvailable:
    case uud::kDalRcConnTimeOut:
    case uud::kDalRcQueryTimeOut:
      return UPLL_RC_ERR_RESOURCE_DISCONNECTED;
    case uud::kDalRcTxnError:
      return UPLL_RC_ERR_GENERIC;
    case uud::kDalRcAccessViolation:
      UPLL_LOG_FATAL("DB access error: %d", result_code);
      return UPLL_RC_ERR_DB_ACCESS;
    case uud::kDalRcParentNotFound:
      return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
    case uud::kDalRcRecordNotFound:
    case uud::kDalRcRecordNoMore:
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    case uud::kDalRcRecordAlreadyExists:
      return UPLL_RC_ERR_INSTANCE_EXISTS;
    case uud::kDalRcConnNotEstablished:
    case uud::kDalRcNotDisconnected:
    case uud::kDalRcInvalidConnHandle:
    case uud::kDalRcInvalidCursor:
    case uud::kDalRcDataError:
    case uud::kDalRcMemoryError:
    case uud::kDalRcInternalError:
    case uud::kDalRcGeneralError:
    default:
      return UPLL_RC_ERR_GENERIC;
  }
}

upll_rc_t MoMgrImpl::UpdateVnodeTables(ConfigKeyVal *ikey,
                                         upll_keytype_datatype_t data_type,
                                         DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    UPLL_LOG_DEBUG("Given Input is Empty ");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  for (int tbl = MAINTBL; tbl < ntable; tbl++) {
    /* Skipping the VTN Main Table During the VTN rename */
    if (GetTable((MoMgrTables)tbl, data_type) != uudst::kDbiVtnTbl &&
        GetTable((MoMgrTables)tbl, data_type) < uudst::kDalNumTables) {
      UPLL_LOG_TRACE("TABLE INDEX IS %d", GetTable((MoMgrTables)tbl, data_type));
      DbSubOp dbop = {kOpNotRead, kOpMatchNone, kOpInOutFlag};
      result_code = UpdateRenameKey(ikey, data_type, UNC_OP_UPDATE,
                        dmi, &dbop, (MoMgrTables)tbl);

      if (UPLL_RC_SUCCESS != result_code &&
          UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG(" UpdateRenameKey Reurn Failure = %d", result_code);
        return result_code;
      }
      UPLL_LOG_TRACE("Updated Successfully in the Table ");
    }
  }
  result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)?
                UPLL_RC_SUCCESS:result_code;
  return result_code;
}



upll_rc_t MoMgrImpl::UpdateRenamedValue(ConfigKeyVal *&rename_info,
                                        DalDmlIntf *dmi,
                                        upll_keytype_datatype_t data_type,
                                        bool &renamed,
                                        bool &no_rename) {
  UPLL_FUNC_TRACE;
  if (!rename_info || (!rename_info->get_key())) {
    UPLL_LOG_DEBUG("Given Input is Empty ");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  ConfigKeyVal *okey = NULL;

  uint8_t rename = 0;

  ConfigKeyVal *combine_key = NULL;
  /* Create the partial key */
  result_code = CopyToConfigKey(okey, rename_info);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("CopyToConfig Return Empty key");
    DELETE_IF_NOT_NULL(okey);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_DEBUG("The CopyToConfigKey out is %s", (okey->ToStrAll()).c_str());
  /* Get the Full key Information */
  DbSubOp dbop = {kOpReadMultiple, kOpMatchNone,
    kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain};
  /* For VTN, we need to read it from Controller Table
   * since while doing VTN rename we are skipping the vtn
   * MAINTBL
   */
  if (UNC_KT_VTN != okey->get_key_type() &&
      UNC_KT_VTN_FLOWFILTER != okey->get_key_type() &&
      UNC_KT_VTN_FLOWFILTER_ENTRY != okey->get_key_type())
    result_code = ReadConfigDB(okey, data_type, UNC_OP_READ, dbop, dmi, MAINTBL);
  else
    result_code = ReadConfigDB(okey, data_type, UNC_OP_READ, dbop, dmi, CTRLRTBL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("ReadConfigDB returns Error = %d ", result_code);
    DELETE_IF_NOT_NULL(okey);
    return result_code;
  }
  ConfigKeyVal *temp = okey;
  while (okey) {
    rename = 0;
    val_rename_vnode *vnode = NULL;
    key_rename_vnode_info  *vnode_rename =
        reinterpret_cast<key_rename_vnode_info *>(rename_info->get_key());
    GET_USER_DATA_FLAGS(okey, rename);
    /* Set the rename flag */
    UPLL_LOG_TRACE("Before Renaming the Rename Flag = %d", rename);
    switch (rename_info->get_key_type()) {
      case UNC_KT_VTN:
        if (!no_rename)
          rename = rename | VTN_RENAME;
        else
          rename = rename & NO_VTN_RENAME;
        UPLL_LOG_TRACE("After the Renaming the Rename Flag = %d ", rename);
        break;
      case UNC_KT_VROUTER:
      case UNC_KT_VLINK:
      case UNC_KT_VBRIDGE:
      case UNC_KT_VTERMINAL:
        if (!no_rename)
          rename = rename | VN_RENAME;
        else
          rename = rename & NO_VN_RENAME;
        UPLL_LOG_TRACE("After the Renaming the Rename Flag = %d ", rename);
        break;
        // Update Here
      case UNC_KT_POLICING_PROFILE:
        switch (table[MAINTBL]->get_key_type()) {
          case UNC_KT_POLICING_PROFILE:
          case UNC_KT_POLICING_PROFILE_ENTRY:
            if (!no_rename)
              rename = rename | PP_RENAME;
            else
              rename = rename & NO_PP_RENAME;
            break;
          case UNC_KT_VTN_POLICINGMAP:
          case UNC_KT_VBR_POLICINGMAP:
          case UNC_KT_VBRIF_POLICINGMAP:
          case UNC_KT_VTERMIF_POLICINGMAP:
            if (!no_rename)
              rename = rename | PM_RENAME;
            else
              rename = rename & NO_PM_RENAME;
            break;
          default:
            break;
        }
        break;
      case UNC_KT_FLOWLIST:
        switch (table[MAINTBL]->get_key_type()) {
          case UNC_KT_FLOWLIST:
          case UNC_KT_FLOWLIST_ENTRY:
            if (!no_rename)
              rename = rename | FL_RENAME;
            else
              rename = rename & NO_FL_RENAME;
            break;
          case UNC_KT_VTN_FLOWFILTER:
          case UNC_KT_VTN_FLOWFILTER_ENTRY:
          case UNC_KT_VTN_FLOWFILTER_CONTROLLER:
          case UNC_KT_VBR_FLOWFILTER:
          case UNC_KT_VBR_FLOWFILTER_ENTRY:
          case UNC_KT_VBRIF_FLOWFILTER:
          case UNC_KT_VBRIF_FLOWFILTER_ENTRY:
          case UNC_KT_VRTIF_FLOWFILTER:
          case UNC_KT_VRTIF_FLOWFILTER_ENTRY:
            if (!no_rename)
              rename = rename | FF_RENAME;
            else
              rename = rename & NO_FF_RENAME;
            break;
          default:
            break;
        }
      default:
        break;
    }
    /* Create the duplicate key by using this function */
    UPLL_LOG_TRACE("The Okey is %s", okey->ToStrAll().c_str());
    result_code = GetChildConfigKey(combine_key, okey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey Return Empty Key ");
     if (combine_key) {
       combine_key->set_next_cfg_key_val(NULL);
       DELETE_IF_NOT_NULL(combine_key);
     }
     DELETE_IF_NOT_NULL(temp);
     return UPLL_RC_ERR_GENERIC;
    }
    /* Set the rename flag */
    SET_USER_DATA_FLAGS(combine_key, rename);
    DumpRenameInfo(rename_info);
    switch (rename_info->get_key_type()) {
      /* fill the vnode if vtn is renamed
       * This part create an etrn in vnode rename table*/
      case UNC_KT_VTN:
        UPLL_LOG_TRACE("Update the Vnode rename tables");
        vnode = reinterpret_cast<val_rename_vnode *>(
                ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
        uuu::upll_strncpy(vnode->ctrlr_vtn_name,
                          vnode_rename->ctrlr_vtn_name,
                          (kMaxLenCtrlrId + 1));
        switch (table[MAINTBL]->get_key_type()) {
          case UNC_KT_VBRIDGE:
            uuu::upll_strncpy(vnode_rename->new_unc_vnode_name,
                              (reinterpret_cast<key_vbr_t *>(okey->get_key()))->vbridge_name,
                              (kMaxLenVnodeName + 1));
            uuu::upll_strncpy(vnode->ctrlr_vnode_name,
                              (reinterpret_cast<key_vbr_t *>(okey->get_key()))->vbridge_name,
                              (kMaxLenVnodeName + 1));
            break;
          case UNC_KT_VROUTER:
            uuu::upll_strncpy(vnode_rename->new_unc_vnode_name,
                              (reinterpret_cast<key_vrt_t *>(okey->get_key()))->vrouter_name,
                              (kMaxLenVnodeName + 1));
            uuu::upll_strncpy(vnode->ctrlr_vnode_name,
                              (reinterpret_cast<key_vrt_t *>(okey->get_key()))->vrouter_name,
                              (kMaxLenVnodeName + 1));
            break;
          case UNC_KT_VLINK:
            uuu::upll_strncpy(vnode_rename->new_unc_vnode_name,
                              (reinterpret_cast<key_vlink_t *>(okey->get_key()))->vlink_name,
                              (kMaxLenVlinkName + 1));
            uuu::upll_strncpy(vnode->ctrlr_vnode_name,
                              (reinterpret_cast<key_vlink_t *>(okey->get_key()))->vlink_name,
                              (kMaxLenVlinkName + 1));
            break;
          case UNC_KT_VBR_POLICINGMAP:
            uuu::upll_strncpy(vnode_rename->new_unc_vnode_name,
                              (reinterpret_cast<key_vbr_t *>(okey->get_key()))->vbridge_name,
                              (kMaxLenVnodeName + 1));
            break;

          case UNC_KT_VBRIF_POLICINGMAP:
            uuu::upll_strncpy(vnode_rename->new_unc_vnode_name,
                              (reinterpret_cast<key_vbr_if *>
                               (okey->get_key()))->vbr_key.vbridge_name,
                              (kMaxLenVnodeName + 1));
            break;
          case UNC_KT_VBR_FLOWFILTER:
            uuu::upll_strncpy(vnode_rename->new_unc_vnode_name,
                              (reinterpret_cast<key_vbr_flowfilter  *>
                               (okey->get_key()))->vbr_key.vbridge_name,
                              (kMaxLenVnodeName + 1));
            break;

          case UNC_KT_VBR_FLOWFILTER_ENTRY:
            uuu::upll_strncpy(vnode_rename->new_unc_vnode_name,
                              (reinterpret_cast<key_vbr_flowfilter_entry  *>
                               (okey->get_key()))->flowfilter_key.vbr_key.vbridge_name,
                              (kMaxLenVnodeName + 1));
            break;
          case UNC_KT_VBRIF_FLOWFILTER:
            uuu::upll_strncpy(vnode_rename->new_unc_vnode_name,
                              (reinterpret_cast<key_vbr_if_flowfilter  *>
                               (okey->get_key()))->if_key.vbr_key.vbridge_name,
                              (kMaxLenVnodeName + 1));
            break;

          case UNC_KT_VBRIF_FLOWFILTER_ENTRY:
            uuu::upll_strncpy(vnode_rename->new_unc_vnode_name,
                              (reinterpret_cast<key_vbr_if_flowfilter_entry *>
                               (okey->get_key()))->flowfilter_key.if_key.vbr_key.vbridge_name,
                              (kMaxLenVnodeName + 1));
            break;
          case UNC_KT_VRTIF_FLOWFILTER:
            uuu::upll_strncpy(vnode_rename->new_unc_vnode_name,
                              (reinterpret_cast<key_vrt_if_flowfilter *>
                               (okey->get_key()))->if_key.vrt_key.vrouter_name,
                              (kMaxLenVnodeName + 1));
            break;
          case UNC_KT_VRTIF_FLOWFILTER_ENTRY:
            uuu::upll_strncpy(vnode_rename->new_unc_vnode_name,
                              (reinterpret_cast<key_vrt_if_flowfilter_entry *>
                               (okey->get_key()))->flowfilter_key.if_key.vrt_key.vrouter_name,
                              (kMaxLenVnodeName + 1));
            break;
          case UNC_KT_VTERMINAL:
            uuu::upll_strncpy(vnode_rename->new_unc_vnode_name,
                              (reinterpret_cast<key_vterm_t *>(okey->get_key()))->vterminal_name,
                              (kMaxLenVnodeName + 1));
            uuu::upll_strncpy(vnode->ctrlr_vnode_name,
                              (reinterpret_cast<key_vterm_t *>(okey->get_key()))->vterminal_name,
                              (kMaxLenVnodeName + 1));
            break;
          case UNC_KT_VTERMIF_POLICINGMAP:
            uuu::upll_strncpy(vnode_rename->new_unc_vnode_name,
                              (reinterpret_cast<key_vterm_if_t *>
                               (okey->get_key()))->vterm_key.vterminal_name,
                              (kMaxLenVnodeName + 1));
            break;

          default:
            break;
        }
        UPLL_LOG_TRACE("The Renamed Value is %d", renamed);
        if (!renamed && !no_rename) {
          /* If VTN is renamed add the entry in vnode and vlink rename table */
          if (table[MAINTBL]->get_key_type() == UNC_KT_VBRIDGE ||
              table[MAINTBL]->get_key_type() == UNC_KT_VROUTER ||
              table[MAINTBL]->get_key_type() == UNC_KT_VTERMINAL ||
              table[MAINTBL]->get_key_type() == UNC_KT_VLINK) {
            ConfigKeyVal *tmp_key = NULL;
            DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain};
            controller_domain ctrlr_dom;
            UPLL_LOG_TRACE("Updating the Rename Table");
            result_code = GetControllerDomainId(okey, &ctrlr_dom);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("GetControllerDomainId Failed");
              free(vnode);
              if (combine_key) {
                combine_key->set_next_cfg_key_val(NULL);
                DELETE_IF_NOT_NULL(combine_key);
              }
              DELETE_IF_NOT_NULL(temp);
              return result_code;
            }
            result_code = GetChildConfigKey(tmp_key, okey);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("GetChildConfigKey Failed");
              free(vnode);
              if (combine_key) {
                combine_key->set_next_cfg_key_val(NULL);
                DELETE_IF_NOT_NULL(combine_key);
              }
              DELETE_IF_NOT_NULL(temp);
              return result_code;
            }
            ConfigVal *cfg_val = new ConfigVal(IpctSt::kIpcInvalidStNum, vnode);
            tmp_key->SetCfgVal(cfg_val);
            SET_USER_DATA_CTRLR_DOMAIN(tmp_key, ctrlr_dom);
            dbop.readop = kOpReadSingle;
            dbop.matchop = kOpMatchCtrlr | kOpMatchDomain;
            result_code = ReadConfigDB(tmp_key, data_type, UNC_OP_READ, dbop,
                                       dmi, RENAMETBL);
            if (UPLL_RC_SUCCESS != result_code &&
                UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
              UPLL_LOG_DEBUG("ReadConfigDB Failed %d", result_code);
              DELETE_IF_NOT_NULL(tmp_key);
              DELETE_IF_NOT_NULL(temp);
              if (combine_key) {
                combine_key->set_next_cfg_key_val(NULL);
                DELETE_IF_NOT_NULL(combine_key);
              }
              return result_code;
            }
            if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
              /* If rename case create an entry in rename table
               * otherwise delete from the rename table */
              vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
              vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

              dbop.readop = kOpNotRead;
              dbop.matchop = kOpMatchNone;
              dbop.inoutop = kOpInOutCtrlr | kOpInOutDomain;
              UPLL_LOG_TRACE("Rename No Rename Falg = %d", no_rename);
              result_code = UpdateConfigDB(tmp_key, data_type, UNC_OP_CREATE,
                                           dmi, &dbop, RENAMETBL);
            }
            DELETE_IF_NOT_NULL(tmp_key);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG(" UpdateConfigDB Failed %d", result_code);
              if (combine_key) {
                combine_key->set_next_cfg_key_val(NULL);
                DELETE_IF_NOT_NULL(combine_key);
              }
              DELETE_IF_NOT_NULL(temp);
              return result_code;
            }
          } else {
            FREE_IF_NOT_NULL(vnode);
          }
        } else {
           FREE_IF_NOT_NULL(vnode);
        }
        break;
      default:
        break;
    }
    if (no_rename) {
      if (table[MAINTBL]->get_key_type() == UNC_KT_VTN ||
          table[MAINTBL]->get_key_type() == UNC_KT_VBRIDGE ||
          table[MAINTBL]->get_key_type() == UNC_KT_VTERMINAL ||
          table[MAINTBL]->get_key_type() == UNC_KT_VROUTER ||
          table[MAINTBL]->get_key_type() == UNC_KT_VLINK ||
          table[MAINTBL]->get_key_type() == UNC_KT_FLOWLIST ||
          table[MAINTBL]->get_key_type() == UNC_KT_POLICING_PROFILE) {
        UPLL_LOG_TRACE("Delete Entry from Rename Table");
        UPLL_LOG_TRACE("Rename Flag is %d", rename);
        if ((table[MAINTBL]->get_key_type() == UNC_KT_VLINK && !(rename & 0X03))
            || !rename) {
          ConfigKeyVal *rename_key = NULL;
          result_code = GetChildConfigKey(rename_key, okey);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("GetChildConfigKey Failed");
            if (combine_key) {
              combine_key->set_next_cfg_key_val(NULL);
              DELETE_IF_NOT_NULL(combine_key);
            }
            DELETE_IF_NOT_NULL(temp);
            return result_code;
          }
          DbSubOp dbop = {kOpNotRead, kOpMatchCtrlr|kOpMatchDomain, kOpInOutNone};
          result_code = UpdateConfigDB(rename_key, data_type, UNC_OP_DELETE,
                                       dmi, &dbop, RENAMETBL);
          if (rename_key)
            DELETE_IF_NOT_NULL(rename_key);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_TRACE("UpdateConfigDB Failed");
            DELETE_IF_NOT_NULL(temp);
            if (combine_key) {
              combine_key->set_next_cfg_key_val(NULL);
              DELETE_IF_NOT_NULL(combine_key);
            }
            return result_code;
          }
        }
      }
    }
    DumpRenameInfo(rename_info);
    /* Add the New name configkeyval to old name configkeyval */
    combine_key->AppendCfgKeyVal(rename_info);
    /* Update the new name into the table */
    DumpRenameInfo(rename_info);
    result_code = UpdateVnodeTables(combine_key, data_type, dmi);
    if (combine_key) {
      combine_key->set_next_cfg_key_val(NULL);
      DELETE_IF_NOT_NULL(combine_key);
    }
    UPLL_LOG_TRACE("Updated in Vnode Tables ");
    if (UPLL_RC_SUCCESS != result_code) {
     DELETE_IF_NOT_NULL(temp);
     return result_code;
    }
    for (int i = 0; i < nchild; i++) {
      unc_key_type_t ktype = child[i];
      /* Avoid the Overlay key types here */
      if (OVERLAY_KT(ktype) || (UNKNOWN_KT(ktype))) {
        UPLL_LOG_TRACE("Key type %d is skipping from rename operation"
                       , ktype);
        continue;
      }
      MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
          const_cast<MoManager *>(GetMoManager(ktype)));
      if (!mgr) {
        UPLL_LOG_DEBUG("Invalid mgr param");
        DELETE_IF_NOT_NULL(temp);
        return UPLL_RC_ERR_GENERIC;
      }
      UPLL_LOG_TRACE("Update Key for the Child key type %d", ktype);
      DumpRenameInfo(rename_info);
      result_code = mgr->UpdateRenamedValue(rename_info, dmi, data_type,
                                            renamed, no_rename);
      if (UPLL_RC_SUCCESS != result_code
          && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        DELETE_IF_NOT_NULL(temp);
        UPLL_LOG_DEBUG("UpdateRenamedValue is Failed %d", result_code);
        return result_code;
      }
    }
    UPLL_LOG_TRACE("The current Instance key type %d return result_code %d",
                   table[MAINTBL]->get_key_type(), result_code);
    okey  = okey->get_next_cfg_key_val();
    UPLL_LOG_TRACE("Fetching Next Record ");
  }
  DELETE_IF_NOT_NULL(temp);
  return result_code;
}

upll_rc_t MoMgrImpl::UpdateTables(IpcReqRespHeader *req,
                                  ConfigKeyVal *&rename_info,
                                  bool &renamed,
                                  DalDmlIntf *dmi, bool &no_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!rename_info) {
    UPLL_LOG_DEBUG("Input key is null");
    return UPLL_RC_ERR_GENERIC;
  }

  switch (rename_info->get_key_type()) {
    case UNC_KT_VTN:
    case UNC_KT_VROUTER:
    case UNC_KT_VBRIDGE:
    case UNC_KT_VTERMINAL:
    case UNC_KT_VLINK:
    case UNC_KT_POLICING_PROFILE:
    case UNC_KT_FLOWLIST:
      /* Update the new name into the tables */
      DumpRenameInfo(rename_info);
      /* Current Instance is Supporting Rename
       * Then Get the Info */
      result_code = UpdateRenamedValue(rename_info, dmi, req->datatype,
                                       renamed, no_rename);
      if (UPLL_RC_SUCCESS != result_code
          && UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("UpdateRenamedValue is Failed %d", result_code);
        return result_code;
      }
      UPLL_LOG_TRACE("UpdateRenamedValue Return code %d", result_code);
      break;
    default:
      break;
  }
  switch (rename_info->get_key_type())  {
    case UNC_KT_VBRIDGE:
    case UNC_KT_VROUTER:
    case UNC_KT_VTERMINAL:
      {
        /* The current instance is VBRIDGE or VROUTER then only
         * call this api
         */
        MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
            const_cast<MoManager*>(GetMoManager(UNC_KT_VLINK)));
        if (!mgr) {
          UPLL_LOG_DEBUG("Invalid mgr");
          return UPLL_RC_ERR_GENERIC;
        }

        DumpRenameInfo(rename_info);
        /* Update the Vnode info in the Vlink table */
        result_code = mgr->UpdateVnodeVal(rename_info, dmi, req->datatype,
                                          no_rename);
        UPLL_LOG_TRACE("The Update Vnode val return value is %d", result_code);

        unc_key_type_t child_key[]= {
          UNC_KT_VBR_FLOWFILTER_ENTRY, UNC_KT_VBRIF_FLOWFILTER_ENTRY,
          UNC_KT_VRTIF_FLOWFILTER_ENTRY, UNC_KT_VTERMIF_FLOWFILTER_ENTRY };

        for (unsigned int i = 0;
             i < sizeof(child_key)/sizeof(child_key[0]); i++) {
          const unc_key_type_t ktype = child_key[i];
          MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
              const_cast<MoManager *>(GetMoManager(ktype)));
          if (!mgr) {
            UPLL_LOG_DEBUG("Instance is NULL");
            return UPLL_RC_ERR_GENERIC;
          }
          DumpRenameInfo(rename_info);
          /* Update the Vnode info in the Vlink table */
          result_code = mgr->UpdateVnodeVal(rename_info, dmi, req->datatype,
                                            no_rename);
          UPLL_LOG_TRACE("The Update Vnode val return value is %d", result_code);
        }
      }
      break;
    case UNC_KT_FLOWLIST:
      {
        unc_key_type_t child_key[]= {
          UNC_KT_POLICING_PROFILE, UNC_KT_POLICING_PROFILE_ENTRY,
          UNC_KT_VTN_FLOWFILTER, UNC_KT_VTN_FLOWFILTER_ENTRY,
          UNC_KT_VBR_FLOWFILTER, UNC_KT_VBR_FLOWFILTER_ENTRY,
          UNC_KT_VBRIF_FLOWFILTER, UNC_KT_VBRIF_FLOWFILTER_ENTRY,
          UNC_KT_VRTIF_FLOWFILTER, UNC_KT_VRTIF_FLOWFILTER_ENTRY,
          UNC_KT_VTERMIF_FLOWFILTER, UNC_KT_VTERMIF_FLOWFILTER_ENTRY};
        for (unsigned int i = 0;
             i < sizeof(child_key)/sizeof(child_key[0]); i++) {
          const unc_key_type_t ktype = child_key[i];
          MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
              const_cast<MoManager *>(GetMoManager(ktype)));
          if (!mgr) {
            UPLL_LOG_DEBUG("Instance is NULL");
            return UPLL_RC_ERR_GENERIC;
          }
          DumpRenameInfo(rename_info);
          /* Update the Vnode info in the Vlink table */
          result_code = mgr->UpdateVnodeVal(rename_info, dmi, req->datatype,
                                            no_rename);
          UPLL_LOG_TRACE("The Update Vnode val return value is %d", result_code);
        }
      }
      break;
    case UNC_KT_POLICING_PROFILE:
      {
        unc_key_type_t child_key[]= {
          UNC_KT_VTN_POLICINGMAP, UNC_KT_VBR_POLICINGMAP,
          UNC_KT_VBRIF_POLICINGMAP, UNC_KT_VTERMIF_POLICINGMAP};
        for (unsigned int i = 0;
             i < sizeof(child_key)/sizeof(child_key[0]); i++) {
          const unc_key_type_t ktype = child_key[i];
          MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
              const_cast<MoManager *>(GetMoManager(ktype)));
          if (!mgr) {
            UPLL_LOG_DEBUG("Instance is NULL");
            return UPLL_RC_ERR_GENERIC;
          }
          DumpRenameInfo(rename_info);
          /* Update the Vnode info in the Vlink table */
          result_code = mgr->UpdateVnodeVal(rename_info, dmi, req->datatype,
                                            no_rename);
          UPLL_LOG_TRACE("The Update Vnode val return value is %d", result_code);
        }
      }
      break;
    default:
      break;
  }
  result_code = (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)?
      UPLL_RC_SUCCESS:result_code;
  return result_code;
}

/*
 *  This function used to chekcs the give node is
 *  unique or not
 */
upll_rc_t MoMgrImpl::VnodeChecks(ConfigKeyVal *ikey,
                                 upll_keytype_datatype_t dt_type,
                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!ikey || !(ikey->get_key()))
    return UPLL_RC_ERR_GENERIC;
  unc_key_type_t nodes[] = {UNC_KT_VBRIDGE, UNC_KT_VROUTER, UNC_KT_VTERMINAL,
                            UNC_KT_VUNKNOWN, UNC_KT_VTEP, UNC_KT_VTUNNEL};
  int nop = sizeof(nodes)/ sizeof(nodes[0]);
  ConfigKeyVal *ck_vnode = NULL;
  UPLL_LOG_TRACE("ikey keytype %d", ikey->get_key_type());
  for (int indx = 0; indx < nop; indx++) {
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
        const_cast<MoManager *>(GetMoManager(nodes[indx])));
    if (!mgr) {
      UPLL_LOG_TRACE("Invalid mgr");
      continue;
    }
    result_code = mgr->CreateVnodeConfigKey(ikey, ck_vnode);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("CreateVnodeConfigKey failed - %d", result_code);
      return result_code;
    }
    result_code = mgr->UpdateConfigDB(ck_vnode, dt_type, UNC_OP_READ,
                                         dmi, MAINTBL);
    if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_TRACE("Existence check in keytype %d result_code  %d",
        ck_vnode->get_key_type(), result_code);
      delete ck_vnode;
      return result_code;
    }
    if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
      if (ikey->get_key_type() != ck_vnode->get_key_type()) {
        UPLL_LOG_INFO("vnode already exists in another vnode tbl");
        result_code = UPLL_RC_ERR_CFG_SEMANTIC;
      }
      delete ck_vnode;
      return result_code;
    }
    if (ck_vnode) {
      delete ck_vnode;
      ck_vnode = NULL;
    }
  }
  if (OVERLAY_KT(ikey->get_key_type()) || UNKNOWN_KT(ikey->get_key_type())) {
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ? UPLL_RC_SUCCESS :
        result_code;
    return result_code;
  }
  VbrIfMoMgr *vbrifmgr = reinterpret_cast<VbrIfMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
  if (vbrifmgr == NULL) {
    UPLL_LOG_DEBUG("Invalid momgr object");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *vbrif_key_val = NULL;

  controller_domain_t ctr_dom;
  memset(&ctr_dom, 0, sizeof(controller_domain_t));
  result_code = GetControllerDomainId(ikey, &ctr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetControllerDomainId failed");
    return result_code;
  }

  if (!ctr_dom.ctrlr || !ctr_dom.domain) {
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctr_dom);
  }

  result_code = vbrifmgr->GetVbrIfFromVExternal(
      reinterpret_cast<key_vnode_t *>
      (ikey->get_key())->vtn_key.vtn_name,
      reinterpret_cast<key_vnode_t *>(ikey->get_key())->vnode_name,
      vbrif_key_val, dmi, ctr_dom, dt_type);
  if (UPLL_RC_SUCCESS == result_code)
    result_code = UPLL_RC_ERR_CFG_SEMANTIC;
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ? UPLL_RC_SUCCESS :
      result_code;
  DELETE_IF_NOT_NULL(vbrif_key_val);
  return result_code;
}

unc_keytype_configstatus_t MoMgrImpl::GetConsolidatedCsStatus(
                             list< unc_keytype_configstatus_t > cs_status) {
  unc_keytype_configstatus_t final_cs_status;
  unc_keytype_configstatus_t current_cs_status;

  list< unc_keytype_configstatus_t >::iterator iter;
  iter = cs_status.begin();
  if (iter == cs_status.end()) {
    return UNC_CS_UNKNOWN;
  } else {
    final_cs_status = *iter;
    if (iter != cs_status.end() && UNC_CS_NOT_SUPPORTED == final_cs_status) {
      return UNC_CS_NOT_APPLIED;
    }
  }

  while (iter != cs_status.end()) {
    current_cs_status = *iter;

    final_cs_status = ComputeStatus(final_cs_status, current_cs_status);
    ++iter;
  }
  return final_cs_status;
}

unc_keytype_configstatus_t MoMgrImpl::ComputeStatus(
    unc_keytype_configstatus_t db_status,
    unc_keytype_configstatus_t cs_status) {
  unc_keytype_configstatus_t result_code;

/* UNC_CS_APPLIED             UNC_CS_INVALID             UNC_CS_INVALID
 * UNC_CS_PARTIALLY_APPLIED   UNC_CS_INVALID             UNC_CS_INVALID
 * UNC_CS_NOT_APPLIED         UNC_CS_INVALID             UNC_CS_INVALID
 * UNC_CS_INVALID             UNC_CS_APPLIED             UNC_CS_INVALID
 * UNC_CS_INVALID             UNC_CS_INVALID             UNC_CS_INVALID
 * UNC_CS_INVALID             UNC_CS_NOT_APPLIED         UNC_CS_INVALID
 * UNC_CS_NOT_SUPPORTED       UNC_CS_INVALID             UNC_CS_INVALID*/
  if (UNC_CS_INVALID == db_status || UNC_CS_INVALID == cs_status) {
    result_code = UNC_CS_INVALID;
  }

/* UNC_CS_APPLIED             UNC_CS_APPLIED             UNC_CS_APPLIED*/

  else  if (UNC_CS_APPLIED == db_status && UNC_CS_APPLIED == cs_status) {
    result_code = UNC_CS_APPLIED;
  }

/* UNC_CS_NOT_APPLIED         UNC_CS_NOT_APPLIED         UNC_CS_NOT_APPLIED
 * UNC_CS_NOT_SUPPORTED       UNC_CS_NOT_APPLIED         UNC_CS_NOT_APPLIED
 * UNC_CS_NOT_APPLIED         UNC_CS_NOT_SUPPORTED       UNC_CS_NOT_APPLIED
 * UNC_CS_NOT_SUPPORTED       UNC_CS_NOT_SUPPORTED       UNC_CS_NOT_APPLIED*/

  else if ((UNC_CS_NOT_APPLIED == db_status &&
           UNC_CS_NOT_APPLIED == cs_status) ||
          (UNC_CS_NOT_SUPPORTED == db_status &&
           UNC_CS_NOT_APPLIED == cs_status) ||
          (UNC_CS_NOT_SUPPORTED == cs_status &&
           UNC_CS_NOT_APPLIED == db_status) ||
          (UNC_CS_NOT_SUPPORTED == cs_status &&
           UNC_CS_NOT_SUPPORTED == db_status)) {
    result_code = UNC_CS_NOT_APPLIED;
  }

/* UNC_CS_APPLIED            UNC_CS_NOT_APPLIED        UNC_CS_PARTIALLY_APPLIED
 * UNC_CS_NOT_APPLIED        UNC_CS_APPLIED            UNC_CS_PARTIALLY_APPLIED
 * UNC_CS_NOT_SUPPORTED      UNC_CS_APPLIED            UNC_CS_PARTIALLY_APPLIED
 * UNC_CS_APPLIED            UNC_CS_NOT_SUPPORTED      UNC_CS_PARTIALLY_APPLIED
 * UNC_CS_PARTIALLY_APPLIED  UNC_CS_APPLIED            UNC_CS_PARTIALLY_APPLIED
 * UNC_CS_APPLIED            UNC_CS_PARTIALLY_APPLIED  UNC_CS_PARTIALLY_APPLIED
 * UNC_CS_NOT_APPLIED        UNC_CS_PARTIALLY_APPLIED  UNC_CS_PARTIALLY_APPLIED
 * UNC_CS_PARTIALLY_APPLIED  UNC_CS_NOT_APPLIED        UNC_CS_PARTIALLY_APPLIED
 * UNC_CS_PARTIALLY_APPLIED  UNC_CS_PARTIALLY_APPLIED  UNC_CS_PARTIALLY_APPLIED*/

  else if ((UNC_CS_APPLIED == db_status ||
           UNC_CS_NOT_APPLIED == db_status ||
           UNC_CS_NOT_SUPPORTED == db_status ||
           UNC_CS_PARTIALLY_APPLIED == db_status) &&
          (UNC_CS_APPLIED == cs_status ||
           UNC_CS_NOT_APPLIED == cs_status ||
           UNC_CS_NOT_SUPPORTED == cs_status ||
           UNC_CS_PARTIALLY_APPLIED == cs_status)) {
    result_code = UNC_CS_PARTIALLY_APPLIED;
  } else {
    result_code = UNC_CS_UNKNOWN;
  }
  return result_code;
}

/* Handles Error translation and Gets UNC Name for the received err_ckv got
 * from driver during TxVote, TxCopyCandidateToRunning & AuditVoteCtrlrStatus*/
upll_rc_t MoMgrImpl::AdaptErrValToVtn(ConfigKeyVal *ckv_drv_rslt,
                                      DalDmlIntf *dmi,
                                      uint8_t* ctrlr_id,
                                      upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  // dt_type is modified only if operation is DELETE, received error
  // not exist in both Rename and Main tbl, Datatype is RUNNING if
  // it is Tx phase else AUDIT durig delete operation

  // 1. For all keytypes except VBR_IF gets UNC Key
  // 2. For VBR_IF keytype if full key exists, gets UNC Key
  //    or get only UNC vtn name
  if (ckv_drv_rslt->get_key_type() != UNC_KT_VBR_IF ||
      (ckv_drv_rslt->get_key_type() == UNC_KT_VBR_IF &&
       IsValidKey(ckv_drv_rslt->get_key(),
                  uudst::vbridge_interface::kDbiVbrName))) {
    result_code = GetRenamedUncKey(ckv_drv_rslt,
                                   dt_type, dmi, ctrlr_id);
    if (UPLL_RC_SUCCESS != result_code &&
        UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
      UPLL_LOG_DEBUG("GetRenamedUncKey failed - %d", result_code);
      return result_code;
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      // If not present in Renametbl check in to Main tbl
      ConfigKeyVal *dup_ckv = NULL;
      result_code = GetChildConfigKey(dup_ckv, ckv_drv_rslt);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_TRACE("GetChildConfigKey failed");
        return result_code;
      }
      DbSubOp dbop = {kOpReadExist, kOpMatchNone, kOpInOutNone};
      result_code = UpdateConfigDB(dup_ckv, dt_type,
                                   UNC_OP_READ, dmi, &dbop);
      DELETE_IF_NOT_NULL(dup_ckv);
      if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS &&
          result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("UpdateConfigDB DB error. result_code = %u", result_code);
        return result_code;
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        // Operation is DELETE, received error not exist in both Rename and Main
        // tbl, Datatype is RUNNING if it is Tx phase
        // else AUDIT durig delete operation
        dt_type = (dt_type == UPLL_DT_CANDIDATE) ? UPLL_DT_RUNNING :UPLL_DT_AUDIT;
        result_code = GetRenamedUncKey(ckv_drv_rslt,
                                       dt_type, dmi, ctrlr_id);
        if (UPLL_RC_SUCCESS != result_code &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
          UPLL_LOG_DEBUG("GetRenamedUncKey failed - %d", result_code);
          return result_code;
        }
      }
    }
  } else {
    // For VBR_IF keytype full key not present get UNC vtn_name
    VtnMoMgr *vtn_mgr = reinterpret_cast<VtnMoMgr *>(
        const_cast<MoManager *>(GetMoManager(UNC_KT_VTN)));
    if (!vtn_mgr) {
      UPLL_LOG_DEBUG("Invalid mgr");
      return UPLL_RC_ERR_GENERIC;
    }
    ConfigKeyVal *vtn_key = NULL;
    result_code = vtn_mgr->GetChildConfigKey(vtn_key, NULL);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed");
      return result_code;
    }
    // Vtn_key should not be null so avoid the null check/
    key_vtn *key = reinterpret_cast<key_vtn_t *>(vtn_key->get_key());
    key_vbr_if *if_key = reinterpret_cast<key_vbr_if_t *>(ckv_drv_rslt->get_key());

    uuu::upll_strncpy(key->vtn_name,if_key->vbr_key.vtn_key.vtn_name,
                      kMaxLenVtnName+1);
    result_code = vtn_mgr->GetRenamedUncKey(vtn_key, dt_type,
                                            dmi, ctrlr_id);
    if ((result_code != UPLL_RC_SUCCESS) &&
        (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
      UPLL_LOG_TRACE("Error in getting renamed name ");
      delete vtn_key;
      return result_code;
    } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      SET_USER_DATA_CTRLR(vtn_key, ctrlr_id);
      DbSubOp dbop = {kOpReadExist, kOpMatchCtrlr, kOpInOutNone};
      result_code = vtn_mgr->UpdateConfigDB(vtn_key, dt_type ,
                                            UNC_OP_READ, dmi, &dbop, CTRLRTBL);
      if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS &&
          result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_ERROR("UpdateConfigDB DB error");
        delete vtn_key;
        return result_code;
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        dt_type = (dt_type == UPLL_DT_CANDIDATE) ? UPLL_DT_RUNNING :UPLL_DT_AUDIT;
        result_code = vtn_mgr->GetRenamedUncKey(vtn_key, dt_type,
                                                dmi, ctrlr_id);
        if (UPLL_RC_SUCCESS != result_code &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
          UPLL_LOG_ERROR("GetRenamedUncKey failed - %d", result_code);
          delete vtn_key;
          return result_code;
        }
      }
    }
    uuu::upll_strncpy(if_key->vbr_key.vtn_key.vtn_name, key->vtn_name,
                      kMaxLenVtnName+1);
    delete vtn_key;
  }
  result_code = UPLL_RC_SUCCESS;
  ConfigVal *cval = ckv_drv_rslt->get_cfg_val();
  switch(ckv_drv_rslt->get_key_type()) {
    case UNC_KT_VBR_IF: {
      // vbr_if_flag is set if err_ckv exists in vbr_if table,
      // this flag used to verify whether it is linked interface
      uint8_t vbrif_flag = false;

      if((IsValidKey(ckv_drv_rslt->get_key(),
                     uudst::vbridge_interface::kDbiVbrName) &&
          IsValidKey(ckv_drv_rslt->get_key(),
                     uudst::vbridge_interface::kDbiIfName))) {
        UPLL_LOG_TRACE("Key is complete");
        vbrif_flag = true;
      } else {
        // Gets vbr_if full key based on vexternal_name in err_ckv
        uint8_t *vext_name;

        val_drv_vbr_if *vbr_if_val = reinterpret_cast<val_drv_vbr_if *>
            (cval->get_val());
        if (IpctSt::kIpcStPfcdrvValVbrIf != cval->get_st_num() ||
            !vbr_if_val || vbr_if_val->valid[PFCDRV_IDX_VEXTERNAL_NAME_VBRIF]
            != UNC_VF_VALID) {
          UPLL_LOG_DEBUG("Vexternal name is not valid. No translation required");
          return UPLL_RC_ERR_NO_SUCH_INSTANCE;
        }
        vext_name = vbr_if_val->vex_name;
        ConfigKeyVal *vbrif_key_val = NULL;
        VbrIfMoMgr *vbrifmgr = reinterpret_cast<VbrIfMoMgr *>
            (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
        if (vbrifmgr == NULL) {
          UPLL_LOG_DEBUG("Invalid momgr object");
          return UPLL_RC_ERR_GENERIC;
        }
        controller_domain_t ctr_dom;
        memset(&ctr_dom, 0, sizeof(controller_domain_t));
        ctr_dom.ctrlr = ctrlr_id;
        result_code = vbrifmgr->GetVbrIfFromVExternal(
            reinterpret_cast<key_vbr_if *>
            (ckv_drv_rslt->get_key())->vbr_key.vtn_key.vtn_name,
            vext_name, vbrif_key_val, dmi, ctr_dom, dt_type);
        // DB Error
        if (result_code != UPLL_RC_SUCCESS &&
            result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_ERROR("Failed to read VBR_IF info");
          DELETE_IF_NOT_NULL(vbrif_key_val);
          return result_code;
        } else if (result_code == UPLL_RC_SUCCESS) {
          // Entry exists in vbr_if table,copies vbr_if full key in ckv_drv_rslt
          memcpy(ckv_drv_rslt->get_key(), vbrif_key_val->get_key(),
                 sizeof(key_vbr_if));
          UPLL_LOG_DEBUG("Driver result updated with logical values");
          DELETE_IF_NOT_NULL(vbrif_key_val);
          vbrif_flag = true;
        } else {
          // If not present in vbr_if tbl check in to vterm_if tbl
          DELETE_IF_NOT_NULL(vbrif_key_val);
          VtermIfMoMgr *vtermif_mgr = reinterpret_cast<VtermIfMoMgr *>(
              const_cast<MoManager*>(GetMoManager(UNC_KT_VTERM_IF)));
          if (!vtermif_mgr) {
            UPLL_LOG_ERROR("Invalid mgr");
            return UPLL_RC_ERR_GENERIC;
          }
          ConfigKeyVal *trans_vtermiferr = NULL;
          result_code = vtermif_mgr->TranslateVbrIfToVtermIfError(
              trans_vtermiferr, ckv_drv_rslt, dt_type, dmi, ctrlr_id);
          // Received error is VTERM_IF error
          if (result_code == UPLL_RC_SUCCESS) {
            if (trans_vtermiferr) {
              ckv_drv_rslt->ResetWithoutNextCkv(trans_vtermiferr);
              DELETE_IF_NOT_NULL(trans_vtermiferr);
            }
          } else  {
            UPLL_LOG_DEBUG("Returning error %d", result_code);
            DELETE_IF_NOT_NULL(trans_vtermiferr);
            return result_code;
          }
        }
      }

      // If received ckv present in vbr_if table means,
      // Checks whether it is linked interface
      if (vbrif_flag) {
        VlinkMoMgr *vlink_mgr = reinterpret_cast<VlinkMoMgr *>(
            const_cast<MoManager *>(GetMoManager(UNC_KT_VLINK)));
        if (!vlink_mgr) {
          UPLL_LOG_DEBUG("Invalid mgr");
          return UPLL_RC_ERR_GENERIC;
        }
        ConfigKeyVal *ck_vlink = NULL;
        vn_if_type iftype;
        SET_USER_DATA_CTRLR(ckv_drv_rslt, ctrlr_id);
        result_code = vlink_mgr->CheckIfMemberOfVlink(ckv_drv_rslt,
                                                      dt_type, ck_vlink, dmi, iftype);
        if (result_code == UPLL_RC_SUCCESS) {
          if ((iftype == kVlinkBoundaryNode1) || (iftype == kVlinkBoundaryNode2)) {
            ckv_drv_rslt->ResetWithoutNextCkv(ck_vlink);
            DELETE_IF_NOT_NULL(ck_vlink);
            UPLL_LOG_TRACE("Vlink Err %s", ckv_drv_rslt->ToStrAll().c_str());
          } else {
            // Internal vlink
            if (ck_vlink) delete ck_vlink;
          }
        } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          // DB Error
          if (ck_vlink) delete ck_vlink;
          return result_code;
        } else {
          // Not a Vlink error
          if (ck_vlink) delete ck_vlink;
          result_code = UPLL_RC_SUCCESS;
        }
      }
    }
    break;
    case UNC_KT_VBR_VLANMAP: {
      ConfigKeyVal *vlanmap_ckv = NULL;
      VlanMapMoMgr *vlanmap_mgr = reinterpret_cast<VlanMapMoMgr *>(
          const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_VLANMAP)));

      result_code = vlanmap_mgr->GetChildConfigKey(vlanmap_ckv, ckv_drv_rslt);
      DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
        kOpInOutFlag | kOpInOutDomain | kOpInOutCtrlr};
      result_code = vlanmap_mgr->ReadConfigDB(vlanmap_ckv, dt_type,
                                              UNC_OP_READ, dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ReadConfigDB from running failed %d", result_code);
        DELETE_IF_NOT_NULL(vlanmap_ckv);
        return result_code;
      }

      ConfigKeyVal *err_ckv = NULL;
      result_code = vlanmap_mgr->TranslateError(&err_ckv, vlanmap_ckv,
                                                dmi, dt_type);
      DELETE_IF_NOT_NULL(vlanmap_ckv);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Returning error %d", result_code);
        if (err_ckv) delete err_ckv;
        return result_code;
      }

      ConfigKeyVal* err_next = ckv_drv_rslt->get_next_cfg_key_val();
      ckv_drv_rslt->set_next_cfg_key_val(NULL);
      ckv_drv_rslt->ResetWith(err_ckv);
      ckv_drv_rslt->AppendCfgKeyVal(err_next);
      DELETE_IF_NOT_NULL(err_ckv);

    }
    default:
    break;
  }

  return result_code;
}

/***********************************************************/
 // SetRedirectNodeAndPortForRead  Api set the redirect_direction
 // As In or OUT and redirect_node & redirect_port in response
 // val_entry structure .
/***********************************************************/
upll_rc_t MoMgrImpl::SetRedirectNodeAndPortForRead
                                   (ConfigKeyVal *ikey,
                                    controller_domain  ctrlr_dom,
                                    val_flowfilter_entry* val_entry,
                                    DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    DbSubOp dbop_up = { kOpReadExist, kOpMatchCtrlr|kOpMatchDomain,
      kOpInOutNone };
    DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr | kOpMatchDomain,
                                                    kOpInOutNone};
    if ((val_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
         UNC_VF_VALID) &&
        (val_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
         UNC_VF_VALID)) {
      ConfigKeyVal *okey = NULL ;
      result_code =  GetRenamedUncRedirectedNode(val_entry, dmi,
                                                 ctrlr_dom.ctrlr);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("SetRenamedNodeAndPortInRead Fail in  - %d",
                       result_code);
        return result_code;
      }

      // Driver does not set the valid bit for [UPLL_IDX_REDIRECT_DIR_FFE]
      // in response val_entry structure so UPLL set it as valid with IN or
      // OUT based on the conditions:
      // 1. redirect_direction as OUT when record exists in vterm_if table
      // 2. redirect_direction as OUT when redirect_node and redirect_interface
      // exists in vbr_if_tbl as vexternal_node and vex_interface
      // 3. If both 1 and 2 are failed then set redirect_direction as IN by
      // assuming that it exists either in vrt_if_tbl or vbr_if_tbl
      val_entry->valid[UPLL_IDX_REDIRECT_DIRECTION_FFE] = UNC_VF_VALID;
      val_entry->redirect_direction = UPLL_FLOWFILTER_DIR_IN;

      VtermIfMoMgr *mgrvtermif = reinterpret_cast<VtermIfMoMgr *>
        (const_cast<MoManager *>(GetMoManager(UNC_KT_VTERM_IF)));
      if (NULL == mgrvtermif) {
        UPLL_LOG_ERROR("Unable to get VTERMIF MoMgr object");
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = mgrvtermif->GetChildConfigKey(okey, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Memory allocation failed for VTERMIF key struct - %d",
                       result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      if (okey == NULL) {
        UPLL_LOG_ERROR("Memory allocation failed for VTERMIF key struct - %d",
                       result_code);
        return result_code;
      }
      key_vterm_if_t *vterm_if_key = static_cast<key_vterm_if_t*>(
          okey->get_key());

      // SetVtnNameInRedirectNodeAndPortForRead will set the vtn_name
      // in vtn_name with respect to requested KTs in ikey
      uint8_t vtn_name[kMaxLenVtnName+1];
      result_code = SetVtnNameInRedirectNodeAndPortForRead(ikey , vtn_name);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("SetVtnNameInRedirectNodeAndPortForRead Fails - %d",
                       result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
       // Fill the vtn_name received from ikey to vterm_if_key
       uuu::upll_strncpy(vterm_if_key->vterm_key.vtn_key.vtn_name,
                         vtn_name, (kMaxLenVtnName + 1));
       uuu::upll_strncpy(vterm_if_key->vterm_key.vterminal_name,
                         val_entry->redirect_node, (kMaxLenVnodeName + 1));
       uuu::upll_strncpy(vterm_if_key->if_name,
                         val_entry->redirect_port, (kMaxLenInterfaceName + 1));
       UPLL_LOG_TRACE("ctrlrid %s, domainid %s",
                      ctrlr_dom.ctrlr, ctrlr_dom.domain);
       SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
       // Check vtnnode and interface exists in table
       result_code = mgrvtermif->UpdateConfigDB(okey, UPLL_DT_RUNNING,
                                   UNC_OP_READ, dmi, &dbop_up, MAINTBL);
       if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
         UPLL_LOG_DEBUG("VTERM_IF in val struct exists in DB");
         val_entry->redirect_direction = UPLL_FLOWFILTER_DIR_OUT;
         DELETE_IF_NOT_NULL(okey);
         return UPLL_RC_SUCCESS;
      } else if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        UPLL_LOG_DEBUG("VTERM_IF does not  exist as the vexternal");
        DELETE_IF_NOT_NULL(okey);
        // USE mgrvbrif here onwards for UNC_KT_VBR_IF
        VbrIfMoMgr *mgrvbrif = reinterpret_cast<VbrIfMoMgr *>
            (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));

        if (NULL == mgrvbrif) {
          UPLL_LOG_ERROR("Unable to get VBRIDGEIF MoMgr object");
          return UPLL_RC_ERR_GENERIC;
        }

        result_code = mgrvbrif->GetChildConfigKey(okey, NULL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Memory allocation failed for VBRIF key struct - %d",
                         result_code);
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
        val_drv_vbr_if *drv_vbr_if_val = reinterpret_cast<val_drv_vbr_if *>
                       (ConfigKeyVal::Malloc(sizeof(val_drv_vbr_if)));

        okey->SetCfgVal(new ConfigVal(IpctSt::kIpcStPfcdrvValVbrIf,
                                      drv_vbr_if_val));
        UPLL_LOG_TRACE("ctrlrid %s, domainid %s",
                       ctrlr_dom.ctrlr, ctrlr_dom.domain);
        UPLL_LOG_TRACE("vexte %s, vexternalif %s",
                       val_entry->redirect_node, val_entry->redirect_port);
        SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);
        // Fill the vtn_name received from ikey to okey key struct
        uuu::upll_strncpy((static_cast<key_vbr_if_t*>(
                    okey->get_key()))->vbr_key.vtn_key.vtn_name,
            vtn_name, kMaxLenVtnName+1);
        uuu::upll_strncpy(drv_vbr_if_val->vex_name,
                          val_entry->redirect_node, (kMaxLenVnodeName + 1));
        uuu::upll_strncpy(drv_vbr_if_val->vex_if_name,
                          val_entry->redirect_port, kMaxLenInterfaceName + 1);
        UPLL_LOG_TRACE("ctrlrid %s, domainid %s",
                       ctrlr_dom.ctrlr, ctrlr_dom.domain);
        SET_USER_DATA_CTRLR_DOMAIN(okey, ctrlr_dom);

        drv_vbr_if_val->valid[PFCDRV_IDX_VEXT_NAME_VBRIF] = UNC_VF_VALID;
        drv_vbr_if_val->valid[PFCDRV_IDX_VEXTIF_NAME_VBRIF] = UNC_VF_VALID;
        result_code = mgrvbrif->ReadConfigDB(okey, UPLL_DT_RUNNING, UNC_OP_READ,
                                             dbop, dmi, MAINTBL);
        if (result_code == UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("VBRIDGE_IF exist as the vexternal - %d",
            result_code);
          key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t *>
              (okey->get_key());

          UPLL_LOG_TRACE("VBR %s, VBRIF %s",
                vbr_if_key->vbr_key.vbridge_name, vbr_if_key->if_name);
          uuu::upll_strncpy(val_entry->redirect_node,
                            vbr_if_key->vbr_key.vbridge_name,
                            kMaxLenVnodeName + 1);
          uuu::upll_strncpy(val_entry->redirect_port,
                            vbr_if_key->if_name,
                            kMaxLenVnodeName + 1);
          val_entry->redirect_direction = UPLL_FLOWFILTER_DIR_OUT;
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          UPLL_LOG_DEBUG("Redirect_direction set as IN as it exists"
                         " either in vrt_if_tbl or vbr_if_tbl");
          val_entry->redirect_direction = UPLL_FLOWFILTER_DIR_IN;
          DELETE_IF_NOT_NULL(okey);
          return UPLL_RC_SUCCESS;
        } else {
          UPLL_LOG_ERROR("ReadConfigDB returns error- %d", result_code);
          DELETE_IF_NOT_NULL(okey);
          return result_code;
        }
      } else {
        UPLL_LOG_ERROR("UpdateConfigDB returns error- %d", result_code);
        DELETE_IF_NOT_NULL(okey);
        return result_code;
      }
      DELETE_IF_NOT_NULL(okey);
    }
    return result_code;
}

/***********************************************************/
// SetVtnNameInRedirectNodeAndPortForRead Api set the vtn name
// in Read/ReadSibling/ReadSiblingMo operation okey depending
// on the keytypes available in the ck_main . it is implemented
// for only POM kts
/***********************************************************/
upll_rc_t MoMgrImpl :: SetVtnNameInRedirectNodeAndPortForRead(
    ConfigKeyVal *ck_main, uint8_t *vnode_vtn_name) {
  UPLL_FUNC_TRACE;
  if (ck_main == NULL) {
    UPLL_LOG_ERROR("Requested okey is NULL ");
    return UPLL_RC_ERR_GENERIC;
  }

  void *main_key = ck_main->get_key();

  switch (ck_main->get_key_type()) {
    case UNC_KT_VBRIF_FLOWFILTER_ENTRY:
      UPLL_LOG_TRACE("Requested Keytype is UNC_KT_VBRIF_FLOWFILTER_ENTRY");
      uuu::upll_strncpy(vnode_vtn_name,
           static_cast<key_vbr_if_flowfilter_entry *>
            (main_key)->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
           kMaxLenVtnName + 1);
      break;

    case UNC_KT_VRTIF_FLOWFILTER_ENTRY:
      UPLL_LOG_TRACE("Requested Keytype is UNC_KT_VRTIF_FLOWFILTER_ENTRY");
      uuu::upll_strncpy(vnode_vtn_name,
           static_cast<key_vrt_if_flowfilter_entry *>
           (main_key)->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
           kMaxLenVtnName + 1);
      break;

    case UNC_KT_VBR_FLOWFILTER_ENTRY:
      UPLL_LOG_TRACE("Requested Keytype is UNC_KT_VBR_FLOWFILTER_ENTRY");
      uuu::upll_strncpy(vnode_vtn_name,
           static_cast<key_vbr_flowfilter_entry*>
           (main_key)->flowfilter_key.vbr_key.vtn_key.vtn_name,
            kMaxLenVtnName + 1);
      break;

    case UNC_KT_VBR_FLOWFILTER:
      UPLL_LOG_TRACE("Requested Keytype is UNC_KT_VBR_FLOWFILTER");
      uuu::upll_strncpy(vnode_vtn_name,
           static_cast<key_vbr_flowfilter *>(
               main_key)->vbr_key.vtn_key.vtn_name,
           kMaxLenVtnName + 1);
      break;

    case UNC_KT_VBRIF_FLOWFILTER:
      UPLL_LOG_TRACE("Requested Keytype is UNC_KT_VBRIF_FLOWFILTER");
      uuu::upll_strncpy(vnode_vtn_name,
          static_cast<key_vbr_if_flowfilter *>(
              main_key)->if_key.vbr_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      break;

    case UNC_KT_VRTIF_FLOWFILTER:
      UPLL_LOG_TRACE("Requested Keytype is UNC_KT_VRTIF_FLOWFILTER");
      uuu::upll_strncpy(vnode_vtn_name,
          static_cast<key_vrt_if_flowfilter *>(
              main_key)->if_key.vrt_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      break;

    case UNC_KT_VTERMIF_FLOWFILTER:
      UPLL_LOG_TRACE("Requested Keytype is UNC_KT_VTERMIF_FLOWFILTER");
      uuu::upll_strncpy(vnode_vtn_name,
          static_cast<key_vterm_if_flowfilter *>(
              main_key)->if_key.vterm_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
        break;

   case UNC_KT_VTERMIF_FLOWFILTER_ENTRY:
     UPLL_LOG_TRACE("Requested Keytype is UNC_KT_VTERMIF_FLOWFILTER_ENTRY");
     uuu::upll_strncpy(vnode_vtn_name,
         static_cast<key_vterm_if_flowfilter_entry *>(
             main_key)->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
         kMaxLenVtnName + 1);
     break;

  default:
     UPLL_LOG_INFO("Unknown keytype %d", ck_main->get_key_type());
     break;
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::GetRenamedUncRedirectedNode(
                                      val_flowfilter_entry*  dup_val_entry,
                                      DalDmlIntf *dmi ,
                                      uint8_t* ctrlr_id) {

  UPLL_FUNC_TRACE;
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal * unc_key = NULL;
  if ((UNC_VF_VALID ==
      dup_val_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE]) &&
      (UNC_VF_VALID ==
      dup_val_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE]) ) {

    unc_key_type_t child_key[]= { UNC_KT_VBRIDGE, UNC_KT_VROUTER ,UNC_KT_VTERMINAL};
    bool isRedirectVnode = false;
    for (unsigned int i = 0;
      i < sizeof(child_key)/sizeof(child_key[0]); i++) {
      const unc_key_type_t ktype = child_key[i];
      MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>(
          const_cast<MoManager *>(GetMoManager(ktype)));
      if (!mgrvbr) {
        UPLL_LOG_TRACE("mgrvbr failed");
        return UPLL_RC_ERR_GENERIC;
      }
      val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode*>
        (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
      if (!rename_val) {
        UPLL_LOG_TRACE("rename_val NULL");
        return UPLL_RC_ERR_GENERIC;
      }

      uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                       dup_val_entry->redirect_node,
                       (kMaxLenVnodeName + 1));
      rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

      result_code = mgrvbr->GetChildConfigKey(unc_key, NULL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey Returned an error");
        if (rename_val) free(rename_val);
        mgrvbr = NULL;
        return result_code;
      }
      SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
      unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_val);
      result_code = mgrvbr->ReadConfigDB(unc_key, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
        RENAMETBL);
      if ((UPLL_RC_SUCCESS != result_code) &&
        (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
        UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
        DELETE_IF_NOT_NULL(unc_key);
        mgrvbr = NULL;
        return result_code;
      }

      if (result_code == UPLL_RC_SUCCESS) {
        if (unc_key->get_key_type() == UNC_KT_VBRIDGE) {
          isRedirectVnode = true;
          key_vbr *vbr_key = reinterpret_cast<key_vbr *>(unc_key->get_key());
          uuu::upll_strncpy(dup_val_entry->redirect_node,
                          vbr_key->vbridge_name,
                          (kMaxLenVnodeName + 1));
        } else if (unc_key->get_key_type() == UNC_KT_VROUTER) {

          isRedirectVnode = true;
          key_vrt *vrt_key = reinterpret_cast<key_vrt *>(unc_key->get_key());
          uuu::upll_strncpy(dup_val_entry->redirect_node,
                           vrt_key->vrouter_name,
                          (kMaxLenVnodeName + 1));
        }  else if (unc_key->get_key_type() == UNC_KT_VTERMINAL) {

          isRedirectVnode = true;
          key_vterm  *vterm_key = reinterpret_cast<key_vterm_t *>(unc_key->get_key());
          uuu::upll_strncpy(dup_val_entry->redirect_node,
                           vterm_key->vterminal_name,
                          (kMaxLenVnodeName + 1));
        }
      }
      DELETE_IF_NOT_NULL(unc_key);
      mgrvbr = NULL;
      if (isRedirectVnode) {
        UPLL_LOG_DEBUG("RedirectVnode is renamed");
        break;
      }
    }
  }
  //DELETE_IF_NOT_NULL(unc_key);
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::CheckVnodeInterfaceForRedirection(ConfigKeyVal *ikey,
                                      ConfigKeyVal *running_ckv,
                                      DalDmlIntf *dmi,
                                      upll_keytype_datatype_t dt_type,
                                      unc_keytype_operation_t op) {

  /* Based on the operation (DELETE or UPDATE) perform the following check
   *  This API should get called for performing semantic validation with
   *  -->op as DELETE if any of the vnode interface is deleted from the candidate
   *  --> op as UPDATE if portmap is deleted for the vbridge interface
   * 1)
   * If op == DELETE then check the given vnode and its interface available in
   * the input ConfigKeyVal(ikey) is referred in the any of the flowfilter
   * entry(
   * VBR_FLOWFILTER_ENTRY,
   * VRT_IF_FLOWFILTER_ENTRY,
   * VBRIF_FLOWFILTER_ENTRY,
   * VTERM_IF_FLOWFILTER_ENTRY) as a redirected vnode interface at candidate
   * configuration
   * if it is present then return UPLL_RC_ERR_SEMANTIC
   *
   * 2)
   * If op == UPDATE then check whether the vbridge
   * interface for which portmap is deleted is referred in the any of
   * the flowfilter entry (
   * VBR_FLOWFILTER_ENTRY,
   * VRT_IF_FLOWFILTER_ENTRY,
   * VBRIF_FLOWFILTER_ENTRY,
   * VTERM_IF_FLOWFILTER_ENTRY) as a redirected vnode interface at candidate
   * configuration
   * if it is presents then return UPLL_RC_ERR_SEMANTIC
   * */
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if ((op == UNC_OP_UPDATE) && (ikey->get_key_type() == UNC_KT_VBR_IF)) {
      if (!ikey->get_cfg_val() || !ikey->get_cfg_val()->get_val()){
        UPLL_LOG_DEBUG("Val structure is NULL");
        return UPLL_RC_ERR_GENERIC;
      }
      val_drv_vbr_if *val_drv_vbrif =
         reinterpret_cast<val_drv_vbr_if *>(GetVal(ikey));
      val_drv_vbr_if *val_drv_vbrif_running =
         reinterpret_cast<val_drv_vbr_if *>(GetVal(running_ckv));

      if(!val_drv_vbrif_running) {
        UPLL_LOG_DEBUG("Val structure for driver is NULL");
        return UPLL_RC_SUCCESS;
      }
      //If portmap is not deleted from candidate  then skip the validation
      //check
      if (!((val_drv_vbrif->vbr_if_val.valid[UPLL_IDX_PM_VBRI] != UNC_VF_VALID) &&
        (val_drv_vbrif_running->vbr_if_val.valid[UPLL_IDX_PM_VBRI] ==
         UNC_VF_VALID))) {
         UPLL_LOG_DEBUG("Do not look up in DB for redirection");
         return UPLL_RC_SUCCESS;
      }
    }

  if (op == UNC_OP_DELETE || op == UNC_OP_UPDATE) {
    unc_key_type_t flowfilter_entry_kts[] = { UNC_KT_VBR_FLOWFILTER_ENTRY,
                                              UNC_KT_VBRIF_FLOWFILTER_ENTRY,
                                              UNC_KT_VRTIF_FLOWFILTER_ENTRY,
                                              UNC_KT_VTERMIF_FLOWFILTER_ENTRY
                                            };

    int nffe_kts = sizeof(flowfilter_entry_kts)/sizeof(flowfilter_entry_kts[0]);
    for (uint8_t count = 0; count < nffe_kts; count++) {
      ConfigKeyVal *ff_entry_key = NULL;
      MoMgrImpl *ff_entry_mgr =
                 reinterpret_cast<MoMgrImpl *>(const_cast<MoManager*>
                   (GetMoManager(flowfilter_entry_kts[count])));
      if (!ff_entry_mgr) {
        UPLL_LOG_INFO("error in fetching MoMgr reference. KT:%d",
                     flowfilter_entry_kts[count]);
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = ff_entry_mgr->GetChildConfigKey(ff_entry_key, NULL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("GetChildConfigKeyFailed. KT %d",
                     flowfilter_entry_kts[count]);
        return result_code;
      }
      val_flowfilter_entry_t *val_ff_entry =
          reinterpret_cast<val_flowfilter_entry_t *>
         (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_t)));
      ff_entry_key->SetCfgVal(new ConfigVal(IpctSt::kIpcStValFlowfilterEntry,
                                            val_ff_entry));
      switch (ikey->get_key_type()) {
        case UNC_KT_VBR_IF:
          AssignVtnVnodeDetailsForRedirection(
                        reinterpret_cast<key_vbr_if *>
                        (ikey->get_key())->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vbr_if *>
                        (ikey->get_key())->vbr_key.vbridge_name,
                        reinterpret_cast<key_vbr_if *>
                        (ikey->get_key())->if_name,
                        ff_entry_key);

          break;
        case UNC_KT_VRT_IF:
          AssignVtnVnodeDetailsForRedirection(
                        reinterpret_cast<key_vrt_if *>
                        (ikey->get_key())->vrt_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vrt_if *>
                        (ikey->get_key())->vrt_key.vrouter_name,
                        reinterpret_cast<key_vrt_if *>
                        (ikey->get_key())->if_name,
                        ff_entry_key);
          break;
        case UNC_KT_VTERM_IF:
          AssignVtnVnodeDetailsForRedirection(
                        reinterpret_cast<key_vterm_if *>
                        (ikey->get_key())->vterm_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vterm_if *>
                        (ikey->get_key())->vterm_key.vterminal_name,
                        reinterpret_cast<key_vterm_if *>
                        (ikey->get_key())->if_name,
                        ff_entry_key);
          break;
        default:
          DELETE_IF_NOT_NULL(ff_entry_key);
          UPLL_LOG_DEBUG("Invalid configkey for redirection check");
          return UPLL_RC_SUCCESS;
      }

      controller_domain ctrlr_dom;
      memset(&ctrlr_dom, 0, sizeof(controller_domain));
      GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
      SET_USER_DATA_CTRLR_DOMAIN(ff_entry_key, ctrlr_dom);
      if(op == UNC_OP_UPDATE) {
        //If Portmap is deleted then lookup into flowfilter entry table
        //for vnode and interface is referred as a redirection with
        //redirect direction as OUT
        val_ff_entry =
          static_cast<val_flowfilter_entry_t *>(
          ff_entry_key->get_cfg_val()->get_val());
        val_ff_entry->valid[UPLL_IDX_REDIRECT_DIRECTION_FFE] = UNC_VF_VALID;
        val_ff_entry->redirect_direction = UPLL_FLOWFILTER_DIR_OUT;
        UPLL_LOG_DEBUG("For Update Operation, Check in FlowFilterEntry tbl"
                       " for redirect-direction out");
      }
      DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain,
                       kOpInOutNone };
      result_code = ff_entry_mgr->UpdateConfigDB(ff_entry_key, dt_type ,
                                                 UNC_OP_READ, dmi,
                                                 &dbop, MAINTBL);
      DELETE_IF_NOT_NULL(ff_entry_key);
      if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
        UPLL_LOG_INFO("Can't delete vnode-interface or Portmap"
                      " as it is referred by  KT %d",
                     flowfilter_entry_kts[count]);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_INFO("UpdateConfigDB failed err %d,", result_code);
        return result_code;
      } else {
        result_code = UPLL_RC_SUCCESS;
      }
    }
  }

  return result_code;
}
void MoMgrImpl::AssignVtnVnodeDetailsForRedirection(uint8_t * vtn_name,
                                uint8_t * vnode_name,
                                uint8_t * if_name,
                                ConfigKeyVal *ffe_key) {

  UPLL_FUNC_TRACE;
  switch (ffe_key->get_key_type()) {
    case UNC_KT_VBR_FLOWFILTER_ENTRY:
         //vtn name
         uuu::upll_strncpy(
         reinterpret_cast<key_vbr_flowfilter_entry_t *>
         (ffe_key->get_key())->flowfilter_key.vbr_key.vtn_key.vtn_name,
         vtn_name,
         kMaxLenVtnName + 1);
    break;
    case UNC_KT_VBRIF_FLOWFILTER_ENTRY:

         uuu::upll_strncpy(
         reinterpret_cast<key_vbr_if_flowfilter_entry_t *>
         (ffe_key->get_key())->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
         vtn_name,
         kMaxLenVtnName + 1);
    break;

    case UNC_KT_VRTIF_FLOWFILTER_ENTRY:
         uuu::upll_strncpy(
         reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
         (ffe_key->get_key())->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
         vtn_name,
         kMaxLenVtnName + 1);
    break;

    case UNC_KT_VTERMIF_FLOWFILTER_ENTRY:
         uuu::upll_strncpy(
         reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
         (ffe_key->get_key())->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
         vtn_name,
         kMaxLenVtnName + 1);
    break;
    default:
         UPLL_LOG_DEBUG("Invalid configkey for redirection check");
     return;
  }

   val_flowfilter_entry_t *val_ff_entry =
        static_cast<val_flowfilter_entry_t *>(
        ffe_key->get_cfg_val()->get_val());

   if (!val_ff_entry) {
      UPLL_LOG_DEBUG("Val structure for ff entry is NULL");
      return;
   }
   //redirect node
   uuu::upll_strncpy(val_ff_entry->redirect_node,
                     vnode_name,
                     (kMaxLenVnodeName + 1));
          //redirect port
   uuu::upll_strncpy(val_ff_entry->redirect_port,
                     if_name,
                     (kMaxLenInterfaceName + 1));
   val_ff_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE] = UNC_VF_VALID;
   val_ff_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE] = UNC_VF_VALID;

}

upll_rc_t MoMgrImpl::PerformSemanticCheckForNWM(
    ConfigKeyVal* ikey,
    DalDmlIntf *dmi ,
    upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;

  if (!ikey || !ikey->get_key()) {
    UPLL_LOG_INFO("input key is null");
    return UPLL_RC_ERR_GENERIC;
  }
  if (!ikey->get_cfg_val() || !ikey->get_cfg_val()->get_val()){
    UPLL_LOG_INFO("Val structure is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  if((ikey->get_key_type() != UNC_KT_VTN_FLOWFILTER_ENTRY) &&
     (ikey->get_key_type() != UNC_KT_VBR_FLOWFILTER_ENTRY) &&
     (ikey->get_key_type() != UNC_KT_VBRIF_FLOWFILTER_ENTRY) &&
     (ikey->get_key_type() != UNC_KT_VRTIF_FLOWFILTER_ENTRY) &&
     (ikey->get_key_type() != UNC_KT_VTERMIF_FLOWFILTER_ENTRY )) {
    return UPLL_RC_SUCCESS;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  val_vtn_flowfilter_entry_t *val_vtn_flowfilter_entry = NULL ;
  val_flowfilter_entry_t *val_flowfilter_entry = NULL ;

  if (ikey->get_key_type() == UNC_KT_VTN_FLOWFILTER_ENTRY)  {
    UPLL_LOG_DEBUG("Requested keytype is VTN_FF_ENTRY");
    val_vtn_flowfilter_entry =
      static_cast<val_vtn_flowfilter_entry_t *>(
          ikey->get_cfg_val()->get_val());
  } else {
    UPLL_LOG_DEBUG("Requested keytype is NOT VTN_FF_ENTRY");
    val_flowfilter_entry =
    static_cast<val_flowfilter_entry_t *>(
        ikey->get_cfg_val()->get_val());
  }

  MoMgrImpl *mgr = NULL;
  mgr = reinterpret_cast<MoMgrImpl *>
       (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_NWMONITOR)));
  if (NULL == mgr) {
    UPLL_LOG_INFO("Unable to get KT_VBR_NWMONITOR momgr");
    return UPLL_RC_ERR_GENERIC;
  }

  // allocate memory for key_nwm key_struct
  ConfigKeyVal *okey = NULL;
  result_code = mgr->GetChildConfigKey(okey, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("GetChildConfigKey failed for NWM - %d", result_code);
    return result_code;
  }

  // fill key_nwm from key/val VBRIF_FLOWFILTER_ENTRY structs
  key_nwm_t *key_nwm = reinterpret_cast<key_nwm_t*>(okey->get_key());
  if (ikey->get_key_type() == UNC_KT_VTN_FLOWFILTER_ENTRY)  {
    if (val_vtn_flowfilter_entry->valid[UPLL_IDX_NWN_NAME_VFFE]
        == UNC_VF_VALID) {
      uuu::upll_strncpy(key_nwm->nwmonitor_name,
                        val_vtn_flowfilter_entry->nwm_name,
                        kMaxLenNwmName+1);
      UPLL_LOG_TRACE("Requested Keytype is UNC_KT_VBr_FLOWFILTER_ENTRY");
      uuu::upll_strncpy(key_nwm->vbr_key.vtn_key.vtn_name,
                        reinterpret_cast<key_vtn_flowfilter_entry_t *>(
                            ikey->get_key())->flowfilter_key.vtn_key.vtn_name,
                        kMaxLenVtnName+1);

      // Check nwm_name exists in table
      result_code = mgr->UpdateConfigDB(okey, dt_type, UNC_OP_READ,
                                        dmi, MAINTBL);
    }
  } else {  // FLOWFILTER_ENTRY other than VTN_FF_ENTRY
    if (val_flowfilter_entry->valid[UPLL_IDX_NWM_NAME_FFE]
        == UNC_VF_VALID) {
      uuu::upll_strncpy(key_nwm->nwmonitor_name,
                        val_flowfilter_entry->nwm_name,
                        kMaxLenNwmName+1);
      if (ikey->get_key_type() == UNC_KT_VBRIF_FLOWFILTER_ENTRY) {
        UPLL_LOG_TRACE("Requested Keytype is UNC_KT_VBRIF_FLOWFILTER_ENTRY");
        uuu::upll_strncpy(key_nwm->vbr_key.vtn_key.vtn_name,
        (reinterpret_cast<key_vbr_if_flowfilter_entry_t *>(ikey->get_key()))->
        flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
        kMaxLenVtnName+1);
      } else if (ikey->get_key_type() == UNC_KT_VRTIF_FLOWFILTER_ENTRY) {
        UPLL_LOG_TRACE("Requested Keytype is UNC_KT_VRTIF_FLOWFILTER_ENTRY");
        uuu::upll_strncpy(key_nwm->vbr_key.vtn_key.vtn_name,
        (reinterpret_cast<key_vrt_if_flowfilter_entry_t *>(ikey->get_key()))->
        flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
        kMaxLenVtnName+1);
      } else if (ikey->get_key_type() == UNC_KT_VBR_FLOWFILTER_ENTRY) {
        UPLL_LOG_TRACE("Requested Keytype is UNC_KT_VBR_FLOWFILTER_ENTRY");
        uuu::upll_strncpy(key_nwm->vbr_key.vtn_key.vtn_name,
        (reinterpret_cast<key_vbr_flowfilter_entry_t *>(ikey->get_key()))->
        flowfilter_key.vbr_key.vtn_key.vtn_name,
        kMaxLenVtnName+1);
      } else if (ikey->get_key_type() == UNC_KT_VTERMIF_FLOWFILTER_ENTRY) {
        UPLL_LOG_TRACE("Requested Keytype is UNC_KT_VTERM_FLOWFILTER_ENTRY");
        uuu::upll_strncpy(key_nwm->vbr_key.vtn_key.vtn_name,
        (reinterpret_cast<key_vterm_if_flowfilter_entry_t *>(ikey->get_key()))->
        flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
        kMaxLenVtnName+1);
      }

      // Check nwm_name exists in table
      result_code = mgr->UpdateConfigDB(okey, dt_type, UNC_OP_READ,
                                        dmi, MAINTBL);
    }
  }

  DELETE_IF_NOT_NULL(okey);
  key_nwm = NULL;

  UPLL_LOG_TRACE("resultcode = %d", result_code);

  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      UPLL_LOG_INFO("Given NWM name does not exist in DB");
      return UPLL_RC_ERR_CFG_SEMANTIC;
    } else if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
      result_code = UPLL_RC_SUCCESS;
    } else {
      UPLL_LOG_INFO("UpdateConfigDB error, resultcode = %d", result_code);
      return result_code;
    }
  }

  UPLL_LOG_TRACE("result code %d.", result_code);
  return result_code;
}

upll_rc_t MoMgrImpl::ValidateNWM(ConfigKeyVal *ikey,
                                 upll_keytype_datatype_t dt_type,
                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ikey) {
    UPLL_LOG_INFO("ConfigKeyVal is NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  unc_key_type_t ff_entry_kts[] = {
    UNC_KT_VTN_FLOWFILTER_ENTRY,
    UNC_KT_VBR_FLOWFILTER_ENTRY,
    UNC_KT_VBRIF_FLOWFILTER_ENTRY,
    UNC_KT_VRTIF_FLOWFILTER_ENTRY,
    UNC_KT_VTERMIF_FLOWFILTER_ENTRY
  };

  int n_ffe_kts = sizeof(ff_entry_kts)/sizeof(ff_entry_kts[0]);
  ConfigKeyVal *ckv_tmp = NULL;
  ConfigVal *cval = NULL;
  for (int indx = 0; indx < n_ffe_kts; indx++) {
    MoMgrImpl *ffe_mgr = reinterpret_cast<MoMgrImpl *>(
        const_cast<MoManager *>(GetMoManager(ff_entry_kts[indx])));
    if (!ffe_mgr) {
      UPLL_LOG_TRACE("Invalid ffe_mgr");
      continue;
    }
    if (ff_entry_kts[indx] == UNC_KT_VTN_FLOWFILTER_ENTRY) {
      val_vtn_flowfilter_entry_t *ffe_val =
          reinterpret_cast<val_vtn_flowfilter_entry_t*>
          (ConfigKeyVal::Malloc(sizeof(val_vtn_flowfilter_entry_t)));
      ffe_val->valid[UPLL_IDX_NWN_NAME_VFFE] = UNC_VF_VALID;
      uuu::upll_strncpy(ffe_val->nwm_name, reinterpret_cast<key_nwm *>
                      (ikey->get_key())->nwmonitor_name, (kMaxLenNwmName + 1));
      cval  = new ConfigVal(IpctSt::kIpcStValVtnFlowfilterEntry, ffe_val);
    } else {
      val_flowfilter_entry_t *ffe_val =
          reinterpret_cast <val_flowfilter_entry_t*>
          (ConfigKeyVal::Malloc(sizeof(val_flowfilter_entry_t)));
      ffe_val->valid[UPLL_IDX_NWM_NAME_FFE] = UNC_VF_VALID;
      uuu::upll_strncpy(ffe_val->nwm_name, reinterpret_cast<key_nwm *>
                      (ikey->get_key())->nwmonitor_name, (kMaxLenNwmName + 1));
      cval  = new ConfigVal(IpctSt::kIpcStValFlowfilterEntry, ffe_val);
    }
    result_code = ffe_mgr->GetChildConfigKey(ckv_tmp, NULL);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("GetChildConfigKey failed - %d", result_code);
      DELETE_IF_NOT_NULL(cval);
      return result_code;
    }
    ckv_tmp->SetCfgVal(cval);

    key_nwm *nwm_key = reinterpret_cast<key_nwm *>(ikey->get_key());
    switch (ff_entry_kts[indx]) {
      case UNC_KT_VTN_FLOWFILTER_ENTRY:
        UPLL_LOG_DEBUG("Request UNC_KT_VTN_FLOWFILTER_ENTRY");
        uuu::upll_strncpy(reinterpret_cast<key_vtn_flowfilter_entry_t*>
            (ckv_tmp->get_key())->flowfilter_key.vtn_key.vtn_name,
            reinterpret_cast<key_nwm *>(nwm_key)->
            vbr_key.vtn_key.vtn_name,(kMaxLenVtnName + 1));
        break;
      case UNC_KT_VBR_FLOWFILTER_ENTRY:
        UPLL_LOG_DEBUG("Request UNC_KT_VBR_FLOWFILTER_ENTRY");
        uuu::upll_strncpy(reinterpret_cast<key_vbr_flowfilter_entry_t *>
            (ckv_tmp->get_key())->flowfilter_key.vbr_key.vtn_key.vtn_name,
            reinterpret_cast<key_nwm *>(nwm_key)->vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName + 1));
         break;
      case UNC_KT_VBRIF_FLOWFILTER_ENTRY:
        UPLL_LOG_DEBUG("Request UNC_KT_VBR_IF_FLOWFILTER_ENTRY");
        uuu::upll_strncpy(reinterpret_cast<key_vbr_if_flowfilter_entry_t *>
           (ckv_tmp->get_key())->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
           reinterpret_cast<key_nwm *>(nwm_key)->vbr_key.vtn_key.vtn_name,
           (kMaxLenVtnName + 1));
        break;
      case UNC_KT_VRTIF_FLOWFILTER_ENTRY:
        UPLL_LOG_DEBUG("Request UNC_KT_VRT_IF_FLOWFILTER_ENTRY");
        uuu::upll_strncpy(reinterpret_cast<key_vrt_if_flowfilter_entry_t *>
            (ckv_tmp->get_key())->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
            reinterpret_cast<key_nwm *>(nwm_key)->vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName + 1));
        break;
      case UNC_KT_VTERMIF_FLOWFILTER_ENTRY:
        UPLL_LOG_DEBUG("Request UNC_KT_VTERM_IF_FLOWFILTER_ENTRY");
        uuu::upll_strncpy(reinterpret_cast<key_vterm_if_flowfilter_entry_t *>
            (ckv_tmp->get_key())->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
            reinterpret_cast<key_nwm *>(nwm_key)->vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName + 1));
        break;
      default:
        UPLL_LOG_DEBUG("KT Requst is not applicable here");
        DELETE_IF_NOT_NULL(ckv_tmp);
        return UPLL_RC_SUCCESS;
    }

    DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
    result_code = ffe_mgr->UpdateConfigDB(ckv_tmp, dt_type, UNC_OP_READ, dmi,
                                          &dbop, MAINTBL);
    delete ckv_tmp;
    ckv_tmp = NULL;
    UPLL_LOG_DEBUG("NWM in FFE: result_code  %d", result_code);
    if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      result_code =  UPLL_RC_SUCCESS;
    } else if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
      UPLL_LOG_INFO("NWM is reffered by KT %d", ff_entry_kts[indx]);
      return UPLL_RC_ERR_CFG_SEMANTIC;
    } else {
      return result_code;
    }
  }
  return result_code;
}

upll_rc_t MoMgrImpl::InitConfigStatus(unc_key_type_t kt,
                                      DalDmlIntf *dmi,
                                      upll_keytype_datatype_t cfg_type) {
  UPLL_FUNC_TRACE;
  for (int tbl = MAINTBL; tbl < ntable; tbl++) {
    const uudst::kDalTableIndex tbl_index = GetTable((MoMgrTables)tbl,
                                                     cfg_type);
    if (tbl_index >= uudst::kDalNumTables)
      continue;
    if (tbl != RENAMETBL) {
      DalBindInfo dal_bind_info(tbl_index);
      upll_rc_t result_code = BindStartup(&dal_bind_info, cfg_type,
                                          (MoMgrTables)tbl);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Bind error %d, KT %d, tbl %d", result_code, kt, tbl);
        return result_code;
      }
      DalResultCode db_result = dmi->UpdateRecords(cfg_type, tbl_index,
                                                   &dal_bind_info);
      if ((db_result != uud::kDalRcSuccess) &&
          (db_result != uud::kDalRcRecordNotFound)) {
         UPLL_LOG_INFO("Update error %d, KT %d, tbl %d", db_result, kt, tbl);
         return DalToUpllResCode(db_result);
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::GlobalTxUpdateController(unc_key_type_t keytype,
                                              uint32_t session_id,
                                              uint32_t config_id,
                                              uuc::UpdateCtrlrPhase phase,
                                              set<string> *affected_ctrlr_set,
                                              DalDmlIntf *dmi,
                                              ConfigKeyVal **err_ckv)  {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode dal_result = uud::kDalRcSuccess;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL, *temp_ck_main = NULL;
  controller_domain ctrlr_dom;  // UNINIT
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle = NULL;
  IpcResponse resp;
  memset(&resp, 0, sizeof(resp));
  unc_keytype_operation_t op = UNC_OP_INVALID;
  //  Gives the operation corresponding to the phase
  result_code = GetOperation(phase, op);
  if (UPLL_RC_SUCCESS != result_code) {
    //  If no operation is done in a particular phase or skipped
    //  UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT is returned.
    if (result_code == UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT) {
      result_code = UPLL_RC_SUCCESS;
    }
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_INFO("GetOperation returns %d", result_code);
    }
    return result_code;
  }
  switch (op) {
    //  Created and Deleted records are retrieved from CTRLR TBL
    case UNC_OP_CREATE:
    case UNC_OP_DELETE:
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op,
                                 req, nreq, &dal_cursor_handle, dmi, CTRLRTBL);
      break;
      //  Updated Records are retrieved from  MAIN TBL
    case UNC_OP_UPDATE:
      result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING, op,
                                 req, nreq, &dal_cursor_handle, dmi, MAINTBL);
      break;
    default:
      UPLL_LOG_TRACE("Invalid operation");
      return UPLL_RC_ERR_GENERIC;
  }
  if (UPLL_RC_SUCCESS != result_code) {
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      result_code = UPLL_RC_SUCCESS;
    }
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(nreq);
    if(dal_cursor_handle != NULL)
      dmi->CloseCursor(dal_cursor_handle, true);
    return result_code;
  }
  resp.header.clnt_sess_id = session_id;
  resp.header.config_id = config_id;
  while (result_code == UPLL_RC_SUCCESS) {
    //  Get Next Record fill the req and nreq configkeyvals.
    dal_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(dal_result);
    if (result_code != UPLL_RC_SUCCESS)
      break;
    ck_main = NULL;
    //  If operation is UNC_OP_DELETE then DB operations are done in
    //  UPLL_DT_RUNNING as the record will exist only in UPLL_DT_RUNNING.
    upll_keytype_datatype_t dt_type = (UNC_OP_DELETE == op)?
        UPLL_DT_RUNNING:UPLL_DT_CANDIDATE;
    if ((op == UNC_OP_CREATE) || (op == UNC_OP_DELETE)) {
      result_code = GetChildConfigKey(ck_main, req);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        return result_code;
      }
      //  req is got from CTRLR TBL so the record is read from MAINTBL.
      DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCs};
      result_code = ReadConfigDB(ck_main, dt_type,
                                 UNC_OP_READ, dbop, dmi, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(ck_main);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        return result_code;
      }
      GET_USER_DATA_CTRLR_DOMAIN(req, ctrlr_dom);
      if (op == UNC_OP_CREATE) {
        // Perform semantic validation and corresponding
        // vexternal/vexternalif conversion on key type specific,
        // before sending to driver.
        bool not_send_to_drv = false;
        result_code = AdaptValToDriver(ck_main, NULL, op,
                                       dt_type, keytype, dmi, not_send_to_drv, false);
        if (result_code != UPLL_RC_SUCCESS) {
          //  If the result_code is UPLL_RC_ERR_CFG_SEMANTIC err_ckv is set.
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(nreq);
          if (UPLL_RC_ERR_CFG_SEMANTIC == result_code ) {
            SET_USER_DATA_CTRLR(ck_main, ctrlr_dom.ctrlr);
            DELETE_IF_NOT_NULL(req);
            *err_ckv = ck_main;
            UPLL_LOG_INFO("AdaptValToDriver Semantic Validation"
                          "failed %d", result_code);
            return result_code;
          }
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(req);
          UPLL_LOG_INFO("AdaptValToDriver failed %d", result_code);
          return result_code;
        }
      }
      temp_ck_main = NULL;
      //  ck_main is cached to set it to err_ckv when driver returns failure.
      result_code = DupConfigKeyVal(temp_ck_main, ck_main, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("DupConfigKeyVal failed %d", result_code);
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(ck_main);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        return result_code;
      }

      SET_USER_DATA_CTRLR_DOMAIN(temp_ck_main, ctrlr_dom);
      //  Rename Key and Val struct to Controller Names.
      result_code = GetRenamedControllerKey(ck_main, dt_type,
                                            dmi, &ctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetRenamedControllerKey failed, err %d", result_code);
        dmi->CloseCursor(dal_cursor_handle, true);
        DELETE_IF_NOT_NULL(ck_main);
        DELETE_IF_NOT_NULL(temp_ck_main);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        return result_code;
      }
      //  Send the request to driver.
      result_code = SendIpcReq(session_id,
                               config_id, op, UPLL_DT_CANDIDATE,
                               ck_main, &ctrlr_dom, &resp);
      affected_ctrlr_set->insert((const char *)ctrlr_dom.ctrlr);
      if (UPLL_RC_ERR_CTR_DISCONNECTED == result_code) {
        result_code = UPLL_RC_SUCCESS;
      }
      if (UPLL_RC_SUCCESS != result_code) {
        break;
      }
    } else if (op == UNC_OP_UPDATE) {
      ConfigKeyVal *ck_ctrlr = NULL;
      result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("DupConfigKeyVal failed %d", result_code);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(req);
        dmi->CloseCursor(dal_cursor_handle, true);
        return result_code;
      }
      void *main = GetVal(ck_main);
      void *val_nrec = (nreq) ? GetVal(nreq) : NULL;
      if (CompareValidValue(main, val_nrec, false)) {
        DELETE_IF_NOT_NULL(ck_main);
        continue;
      }
      result_code = GetChildConfigKey(ck_ctrlr, ck_main);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
        DELETE_IF_NOT_NULL(ck_main);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(req);
        dmi->CloseCursor(dal_cursor_handle, true);
        return result_code;
      }
      //  Method to retrieve the controllers on which the record has been spanned.
      result_code = GetControllerDomainSpan(ck_ctrlr, UPLL_DT_CANDIDATE, dmi);
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        UPLL_LOG_DEBUG("GetControllerDomainSpan UPLL_RC_ERR_NO_SUCH_INSTANCE %d", result_code);
        DELETE_IF_NOT_NULL(ck_ctrlr);
        DELETE_IF_NOT_NULL(ck_main);
        continue;
      } else if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetControllerDomainSpan failed %d", result_code);
        DELETE_IF_NOT_NULL(ck_ctrlr);
        DELETE_IF_NOT_NULL(ck_main);
        DELETE_IF_NOT_NULL(nreq);
        DELETE_IF_NOT_NULL(req);
        dmi->CloseCursor(dal_cursor_handle, true);
        return result_code;
      }

      for (ConfigKeyVal *tmp = ck_ctrlr; tmp != NULL;
           tmp = tmp->get_next_cfg_key_val()) {
        GET_USER_DATA_CTRLR_DOMAIN(tmp, ctrlr_dom);
        bool not_send_to_drv = false;
        result_code = AdaptValToDriver(ck_main, NULL, op,
                                       dt_type, keytype, dmi, not_send_to_drv, false);
        if (result_code != UPLL_RC_SUCCESS) {
          //  If the result_code is UPLL_RC_ERR_CFG_SEMANTIC err_ckv is set.
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(req);
          dmi->CloseCursor(dal_cursor_handle, true);
          if (UPLL_RC_ERR_CFG_SEMANTIC == result_code) {
            SET_USER_DATA_CTRLR(ck_main, ctrlr_dom.ctrlr);
            DELETE_IF_NOT_NULL(ck_ctrlr);
            *err_ckv = ck_main;
            UPLL_LOG_INFO("AdaptValToDriver Semantic Validation" 
                          "failed %d", result_code);
            return result_code;
          }
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ck_ctrlr);
          UPLL_LOG_INFO("AdaptValToDriver failed %d", result_code);
          return result_code;
        }
        //  ck_main is cached to set it to err_ckv when driver returns failure.
        temp_ck_main = NULL;
        result_code = DupConfigKeyVal(temp_ck_main, ck_main, MAINTBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("DupConfigKeyVal failed %d", result_code);
          DELETE_IF_NOT_NULL(ck_ctrlr);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(req);
          dmi->CloseCursor(dal_cursor_handle, true);
          return result_code;
        }
        SET_USER_DATA_CTRLR_DOMAIN(temp_ck_main, ctrlr_dom);

        //  Rename Key and Val struct to Controller Names.
        result_code = GetRenamedControllerKey(ck_main, dt_type,
                                              dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetRenamedControllerKey failed, err %d", result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ck_ctrlr);
          DELETE_IF_NOT_NULL(temp_ck_main);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
        //  Duplicate the value structure from UPLL_DT_RUNNING and append to
        //  configkeyval to be sent to driver.
        ConfigVal *old_cfg = (nreq->get_cfg_val())->DupVal();
        ck_main->AppendCfgVal(old_cfg);
        result_code = SendIpcReq(session_id,
                                 config_id, op, UPLL_DT_CANDIDATE,
                                 ck_main, &ctrlr_dom, &resp);
        affected_ctrlr_set->insert((const char *)ctrlr_dom.ctrlr);
        if (UPLL_RC_ERR_CTR_DISCONNECTED == result_code) {
          result_code = UPLL_RC_SUCCESS;
        }
        if (UPLL_RC_SUCCESS != result_code) {
          break;
        }
        DELETE_IF_NOT_NULL(resp.ckv_data);
        DELETE_IF_NOT_NULL(temp_ck_main);
      }
      DELETE_IF_NOT_NULL(ck_ctrlr);
      if (UPLL_RC_SUCCESS != result_code)
        break;
    }
    DELETE_IF_NOT_NULL(ck_main);
    DELETE_IF_NOT_NULL(temp_ck_main);
    DELETE_IF_NOT_NULL(resp.ckv_data);
  }
  if (UPLL_RC_SUCCESS != result_code) {
    *err_ckv = temp_ck_main;
    DELETE_IF_NOT_NULL(resp.ckv_data);
    DELETE_IF_NOT_NULL(ck_main);
  }
  DELETE_IF_NOT_NULL(nreq);
  DELETE_IF_NOT_NULL(req);
  dmi->CloseCursor(dal_cursor_handle, true);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)?
      UPLL_RC_SUCCESS:result_code;
  return result_code;

}

upll_rc_t MoMgrImpl::CopyRenameTables(const char *ctrlr_id,
                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *temp_ckv = NULL;

  result_code = GetChildConfigKey(temp_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    return result_code;
  }
  SET_USER_DATA_CTRLR(temp_ckv, ctrlr_id);

  const uudst::kDalTableIndex tbl_index = GetTable(RENAMETBL, UPLL_DT_CANDIDATE);
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG(" Invalid Table index - %d", tbl_index);
    delete temp_ckv;
    return UPLL_RC_ERR_GENERIC;
  }

  DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr,
                   kOpInOutDomain|kOpInOutCtrlr };
  DalBindInfo *dal_bind_info = new DalBindInfo(tbl_index);
  result_code = BindAttr(dal_bind_info, temp_ckv, UNC_OP_READ,
                         UPLL_DT_CANDIDATE, dbop, RENAMETBL);

  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("BindAttr failed");
    delete temp_ckv;
    delete dal_bind_info;
    return result_code;
  }
  /*
   * Copy the rename table from candidate to import
   * during partial import operation
   */
  result_code = DalToUpllResCode(dmi->CopyMatchingRecords(UPLL_DT_IMPORT,
                                                          UPLL_DT_CANDIDATE,
                                                          tbl_index,
                                                          dal_bind_info));
  delete temp_ckv;
  delete dal_bind_info;
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("CopyMatchRecords failed %d", result_code);
    return result_code;
  }
  return UPLL_RC_SUCCESS;
}


upll_rc_t MoMgrImpl::PurgeRenameTable(unc_key_type_t keytype,
                                      const char *ctrlr_id,
                                      DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  std::string query_string = "";
  switch(keytype) {
    case UNC_KT_VTN:
     query_string = "DELETE FROM im_vtn_rename_tbl WHERE NOT EXISTS \
                       (SELECT vtn_name from \
                         ( SELECT vtn_name from im_vtn_ctrlr_tbl ) AS temp \
                           WHERE temp.vtn_name = unc_vtn_name );";
     break;
    case UNC_KT_FLOWLIST:
     query_string ="DELETE FROM im_flowlist_rename_tbl WHERE NOT EXISTS \
                    (SELECT flowlist_name from \
                     ( SELECT flowlist_name from im_flowlist_tbl ) AS temp \
                     WHERE temp.flowlist_name = unc_flowlist_name );";
     break;
    case UNC_KT_POLICING_PROFILE:
     query_string = "DELETE FROM im_policingprofile_rename_tbl WHERE NOT EXISTS \
                     (SELECT policingprofile_name from \
                      ( SELECT policingprofile_name from im_policingprofile_tbl ) AS temp \
                      WHERE temp.policingprofile_name = unc_policingprofile_name );";
     break;
    case UNC_KT_VBRIDGE:
      query_string = "DELETE FROM im_vnode_rename_tbl WHERE NOT EXISTS \
                      (select vtn_name, vnode_name from \
                       ( SELECT vtn_name, vbridge_name as vnode_name FROM im_vbr_tbl \
                        UNION \
                        SELECT vtn_name, vrouter_name as vnode_name FROM im_vrt_tbl \
                        UNION \
                        SELECT vtn_name, vterminal_name as vnode_name FROM im_vterminal_tbl) AS temp \
                       WHERE temp.vtn_name=unc_vtn_name AND temp.vnode_name=unc_vnode_name);";
      break;
    case UNC_KT_VLINK:
      query_string = "DELETE FROM im_vlink_rename_tbl WHERE NOT EXISTS \
          (SELECT vtn_name, vlink_name from \
           ( SELECT vtn_name, vlink_name from im_vlink_tbl ) AS temp \
           WHERE temp.vtn_name = unc_vtn_name  and temp.vlink_name = unc_vlink_name);";
      break;
    default:
      return UPLL_RC_ERR_GENERIC;
  }

  const uudst::kDalTableIndex tbl_index = GetTable(RENAMETBL, UPLL_DT_IMPORT);
  if (tbl_index >= uudst::kDalNumTables) {
    UPLL_LOG_DEBUG(" Invalid Table index - %d", tbl_index);
    return UPLL_RC_ERR_GENERIC;
  }

  result_code = DalToUpllResCode(dmi->ExecuteAppQueryModifyRecord(
                                                        UPLL_DT_IMPORT,
                                                        tbl_index, query_string,
                                                        NULL, UNC_OP_DELETE));
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)
    result_code = UPLL_RC_SUCCESS;
  return result_code;
}


#if 0
upll_rc_t MoMgrImpl::PartialMergeValidate(unc_key_type_t keytype,
                                          const char *ctrlr_id,
                                          ConfigKeyVal *conflict_ckv,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  if (!ctrlr_id || !dmi) {
    UPLL_LOG_DEBUG("Invalid input");
    return UPLL_RC_ERR_GENERIC;
  }
  unc_keytype_operation_t op[] = { UNC_OP_DELETE, UNC_OP_CREATE };
  int nop = sizeof(op) / sizeof(op[0]);

  unc_keytype_operation_t op1[] = { UNC_OP_DELETE, UNC_OP_CREATE,
                                   UNC_OP_UPDATE };
  int nop1 = sizeof(op1) / sizeof(op1[0]);

  switch (keytype) {
    case UNC_KT_VTN:
    case UNC_KT_FLOWLIST:
    case UNC_KT_FLOWLIST_ENTRY:
    case UNC_KT_POLICING_PROFILE:
    case UNC_KT_POLICING_PROFILE_ENTRY:
    case UNC_KT_VBR_POLICINGMAP:
    case UNC_KT_VBRIF_POLICINGMAP:
    case UNC_KT_VTERMIF_POLICINGMAP:
    case UNC_KT_VBR_FLOWFILTER:
    case UNC_KT_VBR_FLOWFILTER_ENTRY:
    case UNC_KT_VBRIF_FLOWFILTER:
    case UNC_KT_VBRIF_FLOWFILTER_ENTRY:
    case UNC_KT_VRTIF_FLOWFILTER:
    case UNC_KT_VRTIF_FLOWFILTER_ENTRY:
    case UNC_KT_VTERMIF_FLOWFILTER:
    case UNC_KT_VTERMIF_FLOWFILTER_ENTRY: {
         result_code = MergeValidate(keytype, ctrlr_id, conflict_ckv, dmi);
         return result_code;
    }
    break;
    case UNC_KT_VTN_POLICINGMAP:
        // Validate within IMPORT database for normal and multidomain case
        result_code = PI_MergeValidate_for_Vtn_Policingmap(keytype, ctrlr_id,
                                               conflict_ckv, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
           return result_code;
        }

        // Validate with IMPORT database with RUNNING database
        result_code = ValidateImportWithRunning(keytype, ctrlr_id,
                                            conflict_ckv, op, nop, dmi);
        if ((result_code != UPLL_RC_SUCCESS) &&
            (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
             UPLL_LOG_DEBUG("ValidateImportWithRunning DB err (%d)", result_code);
             return result_code;
        }
         return UPLL_RC_SUCCESS;

    break;
    case UNC_KT_VTN_FLOWFILTER:
        // Validate within IMPORT database for normal and multidomain case
        result_code = PI_MergeValidate_for_Vtn_Flowfilter(keytype, ctrlr_id,
                                               conflict_ckv, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
           return result_code;
        }

        // Validate with IMPORT database with RUNNING database
        result_code = ValidateImportWithRunning(keytype, ctrlr_id,
                                            conflict_ckv, op, nop, dmi);
        if ((result_code != UPLL_RC_SUCCESS) &&
            (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
             UPLL_LOG_DEBUG("ValidateImportWithRunning DB err (%d)", result_code);
             return result_code;
        }
         return UPLL_RC_SUCCESS;
    break;
    case UNC_KT_VTN_FLOWFILTER_ENTRY:
        // Validate within IMPORT database for normal and multidomain case
        result_code = PI_MergeValidate_for_Vtn_Flowfilter_Entry(keytype,
                                    ctrlr_id, conflict_ckv, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
           return result_code;
        }

        // Validate with IMPORT database with RUNNING database
         result_code = ValidateImportWithRunning(keytype, ctrlr_id,
                                            conflict_ckv, op1, nop1, dmi);
         if ((result_code != UPLL_RC_SUCCESS) &&
             (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
             UPLL_LOG_DEBUG("ValidateImportWithRunning DB err (%d)", result_code);
             return result_code;
         }
         return UPLL_RC_SUCCESS;
    break;
    default:
     return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}
#endif

// Note: This function is used to get UNC name for the
// given vNode and vLink.
// This function will differentiate auto rename
// and normal rename
upll_rc_t MoMgrImpl::GetUncKey(ConfigKeyVal *ikey,
                               upll_keytype_datatype_t dt_type,
                               DalDmlIntf *dmi,
                               const char *ctrlr_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  controller_domain ctrlr_dom;
  memset(&ctrlr_dom, 0, sizeof(controller_domain));
//  bool auto_rename  = false;
  result_code = GetControllerDomainId(ikey, &ctrlr_dom);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetControllerDomainId failed");
    return result_code;
  }
  ConfigKeyVal *parent_ckv = NULL;
  result_code  = GetParentConfigKey(parent_ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetParentConfigKey failed");
    return result_code;
  }
  ConfigKeyVal *temp_ckv = NULL;
  if (UNC_KT_VLINK != ikey->get_key_type()) {
    result_code = GetChildConfigKey(temp_ckv, ikey);
    if(UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed");
      delete parent_ckv;
      return result_code;
    }
  } else {
    result_code = DupConfigKeyVal(temp_ckv, ikey, MAINTBL);
    if(UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed");
      delete parent_ckv;
      return result_code;
    }
  }

  key_vtn_t *vtn_key = reinterpret_cast<key_vtn_t *>(parent_ckv->get_key());
  key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t *>(temp_ckv->get_key());

  MoMgrImpl *vtn_mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>
       (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN))));
  if (!vtn_mgr) {
    UPLL_LOG_DEBUG("Instance is null");
    delete parent_ckv;
    DELETE_IF_NOT_NULL(temp_ckv);
    return UPLL_RC_ERR_GENERIC;
  }
  /*
   * Gets the UNC vtn name for given PFC vtn name
   * There is possible VTN Auto rename during partail import
   * so Vnode rename table doesn't have an entry for that VTN
   * so need to get it from import rename table 
   *
   */
  result_code = vtn_mgr->GetRenamedUncKey(parent_ckv, dt_type,
                                          dmi, (uint8_t*)ctrlr_id);
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("GetRenamedUncKey failed %d", result_code);
    delete parent_ckv;
    delete temp_ckv;
    return result_code;
  }
  if (UPLL_RC_SUCCESS == result_code) {
    /*
     * Auto renamed happend for the given vtn
     *
     */
    uint8_t rename = 0x00;
    GET_USER_DATA_FLAGS(ikey, rename);
    rename |= VTN_RENAME;
    SET_USER_DATA_FLAGS(ikey, rename);
    UPLL_LOG_TRACE("The Rename Value in GetUnc %d", rename);
//    auto_rename = true;
  }
  /*
   * Gets the UNC name for the given vnode
   */
  result_code = GetRenamedUncKey(temp_ckv, dt_type, dmi, (uint8_t*)ctrlr_id);
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    delete parent_ckv;
    delete temp_ckv;
    UPLL_LOG_DEBUG("GetRenamedUncKey failed %d", result_code);
    return result_code;
  }
  uint8_t rename = 0x00;
  GET_USER_DATA_FLAGS(ikey, rename);
  UPLL_LOG_TRACE("The Rename Value in GetUnc %d", rename);
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    /*
     * Copy the Auto Rename UNC name into input key
     */
    uuu::upll_strncpy(vbr_key->vtn_key.vtn_name, vtn_key->vtn_name,
                      (kMaxLenVtnName+1));
//    result_code = UPLL_RC_SUCCESS;
  }
  /*
   * Vtn is not auto renamed gets the UNC name from
   * im_vnode_rename_tbl
   */
  SET_USER_DATA(ikey, temp_ckv);
  upll_rc_t res_code = GetChildConfigKey(ikey, temp_ckv);
  if (UPLL_RC_SUCCESS != res_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey failed");
    delete parent_ckv;
    delete temp_ckv;
    return res_code;
  }
  if (UNC_KT_VLINK == ikey->get_key_type()) {
    ikey->SetCfgVal(temp_ckv->get_cfg_val());
    temp_ckv->set_cfg_val(NULL);
  }
  delete parent_ckv;
  delete temp_ckv;
  return result_code;
}



void MoMgrImpl::PrintMap(){
  UPLL_FUNC_TRACE;
  std::map<std::string,std::string>::const_iterator it;
  for (it = auto_rename_.begin(); it != auto_rename_.end(); it++)
   {
     UPLL_LOG_DEBUG("First Name %s Second Name %s", it->first.c_str(),
                    it->second.c_str());
   }
}

// Note: This function will generate the name for name
// conflicting key during partial import and candiadte create

upll_rc_t MoMgrImpl::GenerateAutoName(ConfigKeyVal *&ikey,
                                      upll_keytype_datatype_t dt_type,
                                      controller_domain *ctrlr_dom,
                                      DalDmlIntf *dmi,
                                      bool *auto_rename) {
  UPLL_FUNC_TRACE;
  if (ikey == NULL || dmi == NULL || ctrlr_dom == NULL) {
    UPLL_LOG_INFO("Create error due to insufficient parameters");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  uint8_t *temp_vtn_name = NULL;
  uint8_t *temp_vnode_name = NULL;
  ConfigKeyVal *ck_rename = NULL;
  ConfigKeyVal *parent_ck = NULL;
  uint8_t rename = 0x00;
  struct timeval _timeval;
  struct timezone _timezone;
  gettimeofday(&_timeval, &_timezone);
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr|kOpInOutDomain};
  GET_USER_DATA_FLAGS(ikey, rename);
  /*
   * During Partial Import operation, The Auto rename generates for
   * global keytypes
   */
  if (IS_GLOBAL_KEY_TYPE(ikey->get_key_type()) &&
      UPLL_DT_IMPORT == dt_type) {
    /*
     * The Global key type is exists on controller, so need to
     * rename the global key type
     */
    std::string global_name = reinterpret_cast<const char*>
        (reinterpret_cast<key_vtn_t*>(
                ikey->get_key())->vtn_name);
    if (strlen(global_name.c_str())>=10) {
      global_name.assign(global_name.c_str(), 10);
    }
    /* Renaming the Vnode name based on the Time and Micro seconds */
    std::string global_unc_name = global_name+"_"+
        static_cast<std::ostringstream*>(
            &(std::ostringstream() << _timeval.tv_sec))->str() +
        static_cast<std::ostringstream*>(
            &(std::ostringstream() << _timeval.tv_usec))->str();
    void *val_rename = NULL;
    global_name = reinterpret_cast<const char*>
        (reinterpret_cast<key_vtn_t*>(
                ikey->get_key())->vtn_name);
    /*
     * Store the PFC name first and UNC name second for global keytypes
     */
    auto_rename_.insert(std::pair<std::string, std::string>
                        (global_name, global_unc_name));
    PrintMap();

    switch (ikey->get_key_type()) {
      case UNC_KT_VTN:
        val_rename = reinterpret_cast<val_rename_vtn_t *>(
            ConfigKeyVal::Malloc(sizeof(val_rename_vtn_t)));
        uuu::upll_strncpy(reinterpret_cast<val_rename_vtn_t *>
                          (val_rename)->new_name,
                          reinterpret_cast<key_vtn_t*>(ikey->get_key())->vtn_name,
                          (kMaxLenVtnName+1));
        uuu::upll_strncpy(reinterpret_cast<key_vtn_t *>
                          (ikey->get_key())->vtn_name,
                          global_unc_name.c_str(),
                          (kMaxLenVtnName+1));
        rename |= VTN_RENAME;
        break;
      case UNC_KT_FLOWLIST:
        val_rename = reinterpret_cast
            <val_rename_flowlist *>(ConfigKeyVal::Malloc
                                    (sizeof(val_rename_flowlist)));
        uuu::upll_strncpy(reinterpret_cast<val_rename_flowlist *>(val_rename)
                          ->flowlist_newname,
                          reinterpret_cast<key_flowlist_t*>(ikey->get_key())
                          ->flowlist_name, (kMaxLenFlowListName+1));
        uuu::upll_strncpy(reinterpret_cast<key_flowlist_t *>
                          (ikey->get_key())->flowlist_name,
                          global_unc_name.c_str(),
                          (kMaxLenFlowListName+1));
        rename |= FL_RENAME;
        break;
      case UNC_KT_POLICING_PROFILE:
        val_rename = reinterpret_cast
            <val_rename_policingprofile *>(
                ConfigKeyVal::Malloc(sizeof(val_rename_policingprofile)));
        uuu::upll_strncpy(reinterpret_cast<val_rename_policingprofile *>
                          (val_rename)->policingprofile_newname,
                          reinterpret_cast<key_policingprofile_t*>
                          (ikey->get_key())->policingprofile_name,
                          (kMaxLenPolicingProfileName+1));
        uuu::upll_strncpy(reinterpret_cast<key_policingprofile_t *>
                          (ikey->get_key())->policingprofile_name,
                          global_unc_name.c_str(),
                          (kMaxLenPolicingProfileName+1));
        rename |= PP_RENAME;
        break;
      default:
        return UPLL_RC_ERR_GENERIC;
    }
    SET_USER_DATA_FLAGS(ikey, rename);
    result_code = GetChildConfigKey(ck_rename, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed");
      FREE_IF_NOT_NULL(val_rename);
      return result_code;
    }
    ck_rename->SetCfgVal(new ConfigVal(IpctSt::kIpcInvalidStNum, val_rename));

  } else {
    /* vNode and vLink Auto rename Part
     * This part will deside auto rename required for the given vnode
     * durint partial import or candidate create operation
     */
    /*Check if parent vtn renamed and get the renamed name */
    MoMgrImpl *mgr =
        reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                                      (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN))));
    if (!mgr) {
      UPLL_LOG_DEBUG("Invalid Mgr");
      return UPLL_RC_ERR_GENERIC;
    }
    result_code = GetParentConfigKey(parent_ck, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetParentConfigKey Failed");
      return result_code;
    }
    //  SET_USER_DATA_CTRLR_DOMAIN(parent_ck, *ctrlr_dom);
    UPLL_LOG_DEBUG("ctrlr dom %p %p %p\n", ctrlr_dom->ctrlr, ctrlr_dom->domain,
                   ikey->get_user_data());
    UPLL_LOG_DEBUG("%p %p %p %p \n", parent_ck, ikey, ikey->get_user_data(),
                   parent_ck->get_user_data());
    /*
     * Gets VTN's PFC name
     */
    result_code = mgr->GetRenamedControllerKey(parent_ck, dt_type, dmi,
                                               ctrlr_dom);
    if (result_code != UPLL_RC_SUCCESS) {
      DELETE_IF_NOT_NULL(parent_ck);
      UPLL_LOG_DEBUG("GetRenamedControllerKey failed. Result : %d",
                     result_code);
      return result_code;
    }
    /*
     * Gets the Vtn is renamed  or not from the flag;
     */
    uint8_t vtn_rename = 0x00;
    GET_USER_DATA_FLAGS(parent_ck, vtn_rename);
    rename |= vtn_rename;

    key_vnode* key_rename_vnode = reinterpret_cast<key_vnode*>
        (ConfigKeyVal::Malloc(sizeof(key_vnode)));
    val_rename_vnode_t* val_rename_vnode = reinterpret_cast<val_rename_vnode_t*>
        (ConfigKeyVal::Malloc(sizeof(val_rename_vnode_t)));
    DbSubOp dbop = {kOpReadSingle, kOpMatchCtrlr|kOpMatchDomain, kOpInOutNone};

    if (*auto_rename && UPLL_DT_CANDIDATE == dt_type) {
      /*
       * Checks the vnode exists on the controller or not only for
       * candidate create operation.
       */
      ck_rename = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcInvalidStNum,
                                   key_rename_vnode,
                                   new ConfigVal(IpctSt::kIpcInvalidStNum,
                                                 val_rename_vnode));
      ConfigKeyVal *temp_ckv = NULL;
      result_code = GetChildConfigKey(temp_ckv, ikey);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey failed");
        delete ck_rename;
        delete parent_ck;
        return result_code;
      }
      result_code = ReadConfigDB(temp_ckv, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                                 RENAMETBL);
      if (UPLL_RC_SUCCESS != result_code &&
          UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
        delete temp_ckv;
        delete ck_rename;
        delete parent_ck;
        return result_code;
      }
      if (UPLL_RC_SUCCESS == result_code) {
        /*
         * copy the entry from running to candidate
         */
        val_rename_vnode_t * val_rename_vnode =
            reinterpret_cast<val_rename_vnode_t *>(GetVal(temp_ckv));
        val_rename_vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
        val_rename_vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
        dbop.inoutop = kOpInOutCtrlr | kOpInOutDomain;
        /*
         * Copy the controller vnode name into candidate configruation
         */
        result_code = UpdateConfigDB(temp_ckv, UPLL_DT_CANDIDATE, UNC_OP_CREATE, dmi,
                                     &dbop, RENAMETBL);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
          delete ck_rename;
          delete temp_ckv;
          delete parent_ck;
          return result_code;
        }
        uint8_t rename_flag = 0x00;
        uint8_t vtn_rename = 0x00;
        GET_USER_DATA_FLAGS(parent_ck, vtn_rename);
        GET_USER_DATA_FLAGS(ikey, rename_flag);
        rename_flag |= VN_RENAME;
        rename_flag |= vtn_rename;
        SET_USER_DATA_FLAGS(ikey, rename_flag);
        delete ck_rename;
        delete parent_ck;
        delete temp_ckv;
        return result_code;
      }
      delete temp_ckv;

      uuu::upll_strncpy(val_rename_vnode->ctrlr_vtn_name,
                        reinterpret_cast<key_vtn_t*>
                        (parent_ck->get_key())->vtn_name,
                        (kMaxLenVtnName+1));
      uuu::upll_strncpy(val_rename_vnode->ctrlr_vnode_name,
                        reinterpret_cast<key_vbr_t*>
                        (ikey->get_key())->vbridge_name,
                        (kMaxLenVnodeName+1));

      val_rename_vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
      val_rename_vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
      SET_USER_DATA_CTRLR_DOMAIN(ck_rename, *ctrlr_dom);
      result_code = ReadConfigDB(ck_rename, dt_type, UNC_OP_READ, dbop, dmi,
                                 RENAMETBL);
      if (UPLL_RC_SUCCESS != result_code &&
          UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_ERROR("Record does Not Exists");
        delete ck_rename;
        delete parent_ck;
        return result_code;
      }
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
        /*
         * Checks the new vnode is already renamed in Running configuration
         */
        result_code = ReadConfigDB(ck_rename, UPLL_DT_RUNNING, UNC_OP_READ, dbop, dmi,
                                   RENAMETBL);
        if (UPLL_RC_SUCCESS != result_code &&
            UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
          UPLL_LOG_ERROR("Record does Not Exists");
          delete ck_rename;
          delete parent_ck;
          return result_code;
        }
      }
    }
    if (UPLL_RC_SUCCESS == result_code && *auto_rename) {
      /*
       * The vnode is exists on controller, so need to
       * rename the vnode name
       */
      std::string vnode_name = reinterpret_cast<const char*>
          (reinterpret_cast<key_vbr_t*>(
                  ikey->get_key())->vbridge_name);
      std::string vtn_name = reinterpret_cast<const char*>
          (reinterpret_cast<key_vbr_t*>(
                  ikey->get_key())->vtn_key.vtn_name);
      /*
       * Taking the 5 char from the vtn name
       */
      if (strlen(vtn_name.c_str())>=5) {
        vtn_name.assign(vtn_name.c_str(), 5);
      }
      /*
       * Takin the 10 char from vnode and vlink
       */
      if (strlen(vnode_name.c_str())>=10) {
        vnode_name.assign(vnode_name.c_str(), 10);
      }
      /* Renaming the Vnode name based on the Time and Micro seconds */
      std::string vnode_auto_name = vtn_name+vnode_name+"_"+
          static_cast<std::ostringstream*>(
              &(std::ostringstream() << _timeval.tv_sec))->str() +
          static_cast<std::ostringstream*>(
              &(std::ostringstream() << _timeval.tv_usec))->str();

      if (strlen(vnode_auto_name.c_str()) >= 32) {
        vnode_auto_name.assign(vnode_auto_name.c_str(), 31);
      }
      if (UPLL_DT_IMPORT == dt_type) {

        /*
         * During partial import the auto rename stored in UNC side
         * The controller name not modified. So store the new
         * name into UNC's import configuration
         */
        vnode_name = reinterpret_cast<const char*>
            (reinterpret_cast<key_vbr_t*>(
                    ikey->get_key())->vbridge_name);

        GetVnodeName(ikey, temp_vtn_name, temp_vnode_name);
        /*
         * Move controller vnode name from ikey to vlaue
         */
        uuu::upll_strncpy(val_rename_vnode->ctrlr_vnode_name,
                          temp_vnode_name,
                          (kMaxLenVnodeName+1));
        /*
         * UNC name first and PFC name second
         */
        auto_rename_.insert(std::pair<std::string, std::string>
                            (vnode_auto_name,vnode_name));
        /*
         * Auto generated name mapped to input key during partial import
         * still ikey having unc vtn name only
         */
        uuu::upll_strncpy(temp_vnode_name,
                          vnode_auto_name.c_str(),
                          (kMaxLenVnodeName+1));

        uuu::upll_strncpy(val_rename_vnode->ctrlr_vtn_name,
                          reinterpret_cast<key_vtn_t*>
                          (parent_ck->get_key())->vtn_name,
                          (kMaxLenVtnName+1));
        uuu::upll_strncpy(key_rename_vnode->vnode_name, vnode_auto_name.c_str(),
                          (kMaxLenVnodeName+1));
        uuu::upll_strncpy(key_rename_vnode->vtn_key.vtn_name,
                          temp_vtn_name, (kMaxLenVtnName+1) );
        /*
         * For Import operation ck_rename memory allocated here
         */
        ck_rename = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcInvalidStNum,
                                     key_rename_vnode,
                                     new ConfigVal(IpctSt::kIpcInvalidStNum,
                                                   val_rename_vnode));
        val_rename_vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
        val_rename_vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
        SET_USER_DATA_CTRLR_DOMAIN(ck_rename, *ctrlr_dom);
        rename |= VN_RENAME;
      } else {
        /*
         * Create the new vnode name for the controller, so the new name
         * given to controller. the user given name presents in the
         * UNC candidate configuration
         */
        uuu::upll_strncpy(val_rename_vnode->ctrlr_vnode_name,
                          vnode_auto_name.c_str(),
                          (kMaxLenVnodeName+1));
        uuu::upll_strncpy(val_rename_vnode->ctrlr_vtn_name,
                          reinterpret_cast<key_vtn_t*>
                          (parent_ck->get_key())->vtn_name,
                          (kMaxLenVtnName+1));
        GetVnodeName(ikey, temp_vtn_name, temp_vnode_name);
        uuu::upll_strncpy(key_rename_vnode->vtn_key.vtn_name,
                          temp_vtn_name, (kMaxLenVtnName+1) );
        uuu::upll_strncpy(key_rename_vnode->vnode_name,
                          temp_vnode_name, (kMaxLenVnodeName+1));
        val_rename_vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
        val_rename_vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
        rename |= VN_RENAME;
        /*
         * ck_rename allocated in the above rename table  read operation
         */
      }
    } else {
      /*
       * Vnode is not exists on controller, then checks the
       * VTN is renamed or not. if not renamed then reutrn success
       * otherwise create an entry into the vnode rename table
       */
      if (UPLL_DT_IMPORT == dt_type) {
        uint8_t no_auto_rename = 0x00;
        GET_USER_DATA_FLAGS(ikey, no_auto_rename);
        if (VN_RENAME & no_auto_rename) {
          free (key_rename_vnode);
          free (val_rename_vnode);
          delete parent_ck;
          return UPLL_RC_SUCCESS;
        }
      }
      if (!(rename & RENAME)) {
        DELETE_IF_NOT_NULL(parent_ck);
        if (UPLL_DT_IMPORT == dt_type) {
          free (key_rename_vnode);
          free (val_rename_vnode);
          ConfigKeyVal *temp_ckv = NULL;
          result_code = GetChildConfigKey(temp_ckv, ikey);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("GetChildConfigKey failed");
            return result_code;
          }
          dbop.matchop = kOpMatchCtrlr;
          result_code = UpdateConfigDB(temp_ckv, dt_type, UNC_OP_DELETE,
                                       dmi, &dbop, RENAMETBL);
          delete temp_ckv;
          if (UPLL_RC_SUCCESS != result_code &&
              UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
            UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
            return result_code;
          }
          return UPLL_RC_SUCCESS;
        }
        DELETE_IF_NOT_NULL(ck_rename);
        return UPLL_RC_SUCCESS;
      }
      /*
       * Vnode create an enty into vnode rename table only the
       * vtn auto rename scenario otherwise not required
       *
       */
#if 0
      if (UPLL_DT_IMPORT == dt_type) {
        uint8_t rename = 0x00;
        GET_USER_DATA_FLAGS(ikey, rename);
        if (VTN_RENAME & rename) {
          std::string ctrlr_vtn = (const char*)reinterpret_cast<key_vtn_t *>
              (parent_ck->get_key())->vtn_name;
          std::map<std::string, std::string>::iterator ctrlr_map =
              auto_rename_.find(ctrlr_vtn);
          if (ctrlr_map != auto_rename_.end()) {
            // Continue and create an entry into
            // rename table
          } else {
            free (key_rename_vnode);
            free (val_rename_vnode);
            delete parent_ck;
            return UPLL_RC_SUCCESS;

          }
        }
      }
#endif
      // create entry in Vnode Rename Table-parent_ck contains the renamed name
      // TODO (karthi) Here removed CreateVnodeRenameEntry and combined
      GetVnodeName(ikey, temp_vtn_name, temp_vnode_name);
      if (temp_vtn_name == NULL || temp_vnode_name == NULL) {
        FREE_IF_NOT_NULL(key_rename_vnode);
        FREE_IF_NOT_NULL(val_rename_vnode);
        return UPLL_RC_ERR_GENERIC;
      }
      uuu::upll_strncpy(key_rename_vnode->vtn_key.vtn_name,
                        temp_vtn_name, (kMaxLenVtnName+1) );
      uuu::upll_strncpy(key_rename_vnode->vnode_name,
                        temp_vnode_name, (kMaxLenVnodeName+1));

      uuu::upll_strncpy(val_rename_vnode->ctrlr_vtn_name,
                        reinterpret_cast<key_vtn_t*>(parent_ck->get_key())->vtn_name,
                        (kMaxLenVtnName+1));
      uuu::upll_strncpy(val_rename_vnode->ctrlr_vnode_name,
                        temp_vnode_name, (kMaxLenVnodeName+1));

      if (UPLL_DT_IMPORT == dt_type) {
        ck_rename = new ConfigKeyVal(UNC_KT_VBRIDGE, IpctSt::kIpcInvalidStNum,
                                     key_rename_vnode,
                                     new ConfigVal(IpctSt::kIpcInvalidStNum, val_rename_vnode));
      }

      val_rename_vnode->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
      val_rename_vnode->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

      SET_USER_DATA_CTRLR_DOMAIN(ck_rename, *ctrlr_dom);
      rename |= VTN_RENAME;
    }
  }
  /* Update the Rename Flag for vnode in the main table */
  SET_USER_DATA_FLAGS(ikey, rename);
  /* Create the Entry in Vnode rename table for vbr and vrt
   * Vlink rename table for vlink key type */
  dbop.matchop = kOpMatchNone;
  dbop.inoutop = kOpInOutCtrlr|kOpInOutDomain;
  result_code = UpdateConfigDB(ck_rename, dt_type, UNC_OP_CREATE, dmi, &dbop,
                               RENAMETBL);
  if (UPLL_DT_IMPORT == dt_type) {
    if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code) {
      UPLL_LOG_DEBUG("During Partial Import vNode is modified across \
                     the domain");
      result_code = UPLL_RC_SUCCESS;
    }
  }
  DELETE_IF_NOT_NULL (ck_rename);
  DELETE_IF_NOT_NULL(parent_ck);
  return result_code;
}

upll_rc_t MoMgrImpl::CreateVtn(ConfigKeyVal *ikey,
                               upll_keytype_datatype_t dt_type,
                               DalDmlIntf *dmi,
                               bool *is_rename) {
  UPLL_FUNC_TRACE;
  ConfigKeyVal *parent_ckv = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  bool auto_rename = false;
  result_code = GetParentConfigKey(parent_ckv, ikey);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetParentConfigKey failed");
    return result_code;
  }
  MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>
      (const_cast<MoManager *>
      (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN))));
  if (!mgr) {
    UPLL_LOG_DEBUG("Instance is null");
    delete parent_ckv;
    return UPLL_RC_ERR_GENERIC;
  }
  uint8_t *ctrlr_id = NULL;
  uint8_t rename = 0x00;
  SET_USER_DATA_FLAGS(parent_ckv, NO_RENAME);
  GET_USER_DATA_CTRLR(ikey, ctrlr_id);
  result_code = mgr->GetRenamedUncKey(parent_ckv, UPLL_DT_CANDIDATE, dmi,
                                        ctrlr_id);
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_DEBUG("GetRenamedUncKey failed %d", result_code);
    delete parent_ckv;
    return result_code;
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
    /*
     * Check VTN is auto renamed or not
     */
    const char *ctrlr_vtn_name = (const char *)reinterpret_cast<key_vtn_t *>
                            (parent_ckv->get_key())->vtn_name;
    std::map<std::string, std::string>::iterator iter_ =
        auto_rename_.find(ctrlr_vtn_name);
    if (iter_ != auto_rename_.end()) {
      *is_rename = true;
      UPLL_LOG_TRACE("TODO The auto rename for PFC %s -> UNC %s",
                     iter_->first.c_str(), iter_->second.c_str());
      uuu::upll_strncpy(reinterpret_cast<key_vtn_t*>(parent_ckv->get_key())->vtn_name,
                        iter_->first.c_str(), (kMaxLenVtnName+1));
    }
    if (!(*is_rename)) {
      result_code = mgr->AutoRename(parent_ckv, dt_type, dmi, &auto_rename);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("AutoRename failed %d", result_code);
        delete parent_ckv;
        return result_code;
      }
    }
  }else {
    *is_rename = true;
  }
  GET_USER_DATA_FLAGS(parent_ckv, rename);
  rename &= VTN_RENAME;
  SET_USER_DATA_FLAGS(ikey, rename);
  result_code = mgr->UpdateConfigDB(parent_ckv, dt_type,
                                    UNC_OP_CREATE, dmi, MAINTBL);
  /*
   * Multi-domain scenario same vtn try to create in
   * VTN main table so for avoiding  skipping the exists command
   */
  if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code)
    result_code = UPLL_RC_SUCCESS;
  if (UPLL_RC_SUCCESS != result_code) {
    delete parent_ckv;
    return result_code;
  }
  if (*is_rename) {
    ConfigKeyVal *temp_ckv = NULL;
    result_code = mgr->GetChildConfigKey(temp_ckv, parent_ckv);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed");
      delete parent_ckv;
      return result_code;
    }
    controller_domain ctrlr_dom;
    memset(&ctrlr_dom, 0, sizeof(controller_domain));
    GET_USER_DATA_CTRLR_DOMAIN(parent_ckv, ctrlr_dom);

    val_rename_vtn *rename_vtn = reinterpret_cast<val_rename_vtn *>(
        ConfigKeyVal::Malloc(sizeof(val_rename_vtn)));
    rename_vtn->valid[UPLL_IDX_NEW_NAME_RVTN] = UNC_VF_VALID;

    /*
     * The auto rename presents in import rename table
     * no need to create again so just return success
     */
    const char *unc_vtn_name = (const char *)reinterpret_cast<key_vtn_t *>
        (temp_ckv->get_key())->vtn_name;
    std::map<std::string, std::string>::iterator iter = auto_rename_.find(unc_vtn_name);
    if (iter != auto_rename_.end()) {
      UPLL_LOG_TRACE("TODO The auto rename for PFC %s -> UNC %s",
                     iter->first.c_str(), iter->second.c_str());
      uuu::upll_strncpy(reinterpret_cast<key_vtn_t*>(temp_ckv->get_key())->vtn_name,
                        iter->first.c_str(), (kMaxLenVtnName+1));
    } else {
      uuu::upll_strncpy(reinterpret_cast<key_vtn_t*>(temp_ckv->get_key())->vtn_name,
                        reinterpret_cast<key_vtn_t *>(ikey->get_key())->vtn_name,
                        (kMaxLenVtnName+1));
    }
    uuu::upll_strncpy(rename_vtn->new_name,
                      reinterpret_cast<key_vtn_t *>(temp_ckv->get_key())->vtn_name,
                     (kMaxLenVtnName+1));
    parent_ckv->SetCfgVal(new ConfigVal(IpctSt::kIpcInvalidStNum, rename_vtn));
    DbSubOp dbop = { kOpReadSingle, kOpMatchNone,
                    kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag };
    result_code = mgr->UpdateConfigDB(parent_ckv, UPLL_DT_IMPORT,
                                    UNC_OP_CREATE, dmi, &dbop, RENAMETBL);
    if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code)
      result_code = UPLL_RC_SUCCESS;
    delete temp_ckv;
  }
  if (auto_rename) {
    *is_rename = auto_rename;
  }
  delete parent_ckv;
  return result_code;
}

upll_rc_t MoMgrImpl::AutoRename(ConfigKeyVal *ikey,
                                upll_keytype_datatype_t dt_type,
                                DalDmlIntf *dmi,
                                bool *is_rename) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

   if (ISRENAME_KEYTYPE(ikey->get_key_type())) {
    ConfigKeyVal *unc_ckv = NULL;
    result_code = GetChildConfigKey(unc_ckv, ikey);
    if (UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed");
      return result_code;
    }
    DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr,
                    kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag };
    controller_domain ctrlr_dom;
    memset(&ctrlr_dom, 0, sizeof(controller_domain));
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctrlr_dom);
    /*
     * Check existence on same controller or not
     */
    if (UNC_KT_VLINK == ikey->get_key_type()) {
      SET_USER_DATA_CTRLR_DOMAIN(ikey->get_cfg_val(), ctrlr_dom);
      val_vlink_t *vlink_val = reinterpret_cast<val_vlink_t *>(
               ConfigKeyVal::Malloc(sizeof(val_vlink_t)));
      unc_ckv->SetCfgVal(new ConfigVal(IpctSt::kIpcStValVlink, vlink_val));
      SET_USER_DATA_CTRLR(unc_ckv->get_cfg_val(), ctrlr_dom.ctrlr);
    }
    MoMgrTables tbl = MAINTBL;
    if (UNC_KT_VTN == ikey->get_key_type()||
        UNC_KT_FLOWLIST == ikey->get_key_type() ||
        UNC_KT_POLICING_PROFILE == ikey->get_key_type()) {
      tbl = CTRLRTBL;
      dbop.matchop = kOpMatchCtrlr|kOpMatchDomain;
    }
    result_code = ReadConfigDB(unc_ckv, UPLL_DT_CANDIDATE,
                               UNC_OP_READ, dbop, dmi, tbl);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE!= result_code &&
        UPLL_RC_SUCCESS != result_code) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      delete unc_ckv;
      return result_code;
    }
    /*
     * If record is exists then need to compare the
     * rename flag and vlaues
     * otherwise continue
     */
    if (UPLL_RC_SUCCESS == result_code) {
      /*
       * Compare the rename flag for candidate
       * and imported configuration
       * if flag is matched then
       *  do the auto rename otherwise normal create
       */
      uint8_t ctrlr_rename_flag = 0x00;
      GET_USER_DATA_FLAGS(ikey, ctrlr_rename_flag);
      uint8_t unc_rename_flag = 0x00;
      GET_USER_DATA_FLAGS(unc_ckv, unc_rename_flag);
      UPLL_LOG_TRACE("TODO rename flag values getrenamed %d and readconfig %d",
                     ctrlr_rename_flag, unc_rename_flag);
      /*
       * Flag value is same may be give vtn is not renamed in UNC
       * or new VTN name
       */
      if (UNC_KT_VLINK == ikey->get_key_type()) {
        uint8_t ca_flag = unc_rename_flag & RENAME_BITS;
        uint8_t im_flag = ctrlr_rename_flag & RENAME_BITS;
        if (ca_flag == im_flag) {
          *is_rename = false;
        }else {
          *is_rename = true;
        }
        UPLL_LOG_TRACE("TODO rename flag values for vlink %d and readconfig %d",
                       ca_flag, im_flag);
      } else {
        if (unc_rename_flag == ctrlr_rename_flag) {
          /*
           * Checking other keytype renamed same name earlier
           */
          *is_rename = false;
        } else {
          *is_rename = true;
        }
      }
    } else if (UNC_KT_VBRIDGE == unc_ckv->get_key_type() ||
               UNC_KT_VROUTER == unc_ckv->get_key_type() ||
               UNC_KT_VTERMINAL == unc_ckv->get_key_type()) {
      unc_ckv->SetCfgVal(NULL);
      /* Checking some other vnode renamed earlier
       */
      result_code = ReadConfigDB(unc_ckv, UPLL_DT_IMPORT,
                                 UNC_OP_READ, dbop, dmi, RENAMETBL);
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE!= result_code &&
          UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
        delete unc_ckv;
        return result_code;
      }
      if (UPLL_RC_SUCCESS == result_code) {
        *is_rename = true;
      } else {
        *is_rename = false;
      }
    }
    if (*is_rename) {
      /* Set the auto rename in unc for that vnode
       * and create an entry in vnode rename table
       */
      result_code = GenerateAutoName (ikey,
                                      dt_type,
                                      &ctrlr_dom,
                                      dmi,
                                      is_rename);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GenerateAutoName failed %d", result_code);
        delete unc_ckv;
        return result_code;
      }
    }
    delete unc_ckv;
  } else  {
    uint8_t *ctrlr_id = NULL;
    GET_USER_DATA_CTRLR(ikey, ctrlr_id);
    /* (TOCHECK)Karthi
     *  vnode childrens refer the DT_IMPORT rename
     * table because possible of vnode is auto renamed
     */
    if ((ikey->get_key_type() == UNC_KT_VBR_FLOWFILTER_ENTRY)   ||
        (ikey->get_key_type() == UNC_KT_VBRIF_FLOWFILTER_ENTRY) ||
        (ikey->get_key_type() == UNC_KT_VRTIF_FLOWFILTER_ENTRY) ||
        (ikey->get_key_type() == UNC_KT_VTERMIF_FLOWFILTER_ENTRY)) {
      result_code = GetRenamedUncKeyWoRedirection(ikey,
                            dt_type, dmi, (uint8_t *) ctrlr_id);
      if (UPLL_RC_SUCCESS != result_code &&
         UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
         UPLL_LOG_DEBUG("GetRenamedUncKey failed %d", result_code);
         return result_code;
      }
    } else {
      result_code = GetRenamedUncKey(ikey, dt_type, dmi,
                                   (uint8_t *)ctrlr_id);
      if (UPLL_RC_SUCCESS != result_code &&
         UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
         UPLL_LOG_DEBUG("GetRenamedUncKey failed %d", result_code);
         return result_code;
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::ValidateImportWithRunning(unc_key_type_t keytype,
                                               const char *ctrlr_id, ConfigKeyVal *ikey,
                                               unc_keytype_operation_t op[],
                                               int nop, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL;
  ConfigKeyVal *ckv = NULL, *vtn_ckv = NULL;
  DalCursor *dal_cursor_handle = NULL;
  DalResultCode dal_result = uud::kDalRcSuccess;
  MoMgrImpl *mgr = NULL;

  switch (keytype) {
    case UNC_KT_FLOWLIST:
    case UNC_KT_FLOWLIST_ENTRY:
      mgr = reinterpret_cast<MoMgrImpl *>
          (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
      break;
    case UNC_KT_POLICING_PROFILE_ENTRY:
      mgr = reinterpret_cast<MoMgrImpl *>
          (const_cast<MoManager *> (GetMoManager(UNC_KT_POLICING_PROFILE)));
      break;
    case UNC_KT_VTN_FLOWFILTER:
    case UNC_KT_VTN_FLOWFILTER_ENTRY:
    case UNC_KT_VTN_POLICINGMAP:
      mgr = reinterpret_cast<MoMgrImpl *>
          (const_cast<MoManager *> (GetMoManager(UNC_KT_VTN)));
      break;
    default:
      break;  
  }
  // Check the diff records and validate the records
  for (int i = 0; i < nop; i++) {
    dal_cursor_handle = NULL;
    result_code = DiffConfigDB(UPLL_DT_IMPORT, UPLL_DT_RUNNING, op[i], req,
                               nreq, &dal_cursor_handle, dmi, NULL, MAINTBL);

    while (result_code == UPLL_RC_SUCCESS) {
      dal_result = dmi->GetNextRecord(dal_cursor_handle);
      result_code = DalToUpllResCode(dal_result);
      if ((result_code != UPLL_RC_SUCCESS) &&
          (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
        UPLL_LOG_ERROR("DB error %d result code %d", op[i], result_code);
        DELETE_IF_NOT_NULL(req);
        DELETE_IF_NOT_NULL(nreq);
        if (dal_cursor_handle) {
          dmi->CloseCursor(dal_cursor_handle, true);
          dal_cursor_handle = NULL;
        }
        return result_code;
      } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("No Instance operation %d result code %d", op[i], result_code);
        break;
      } else if (result_code == UPLL_RC_SUCCESS) {
        ck_main = NULL;

        if ((UNC_OP_DELETE == op[i]) &&
            (UNC_KT_VTN_FLOWFILTER == keytype ||
             UNC_KT_VTN_FLOWFILTER_ENTRY == keytype ||
             UNC_KT_VTN_POLICINGMAP == keytype ||
             UNC_KT_FLOWLIST_ENTRY == keytype ||
             UNC_KT_POLICING_PROFILE_ENTRY)) {

          result_code = mgr->GetChildConfigKey(vtn_ckv, NULL);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("GetChildConfigKey failed err code(%d)", result_code);
            if (dal_cursor_handle)
              dmi->CloseCursor(dal_cursor_handle, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            return result_code;
          }
          if (keytype == UNC_KT_VTN_FLOWFILTER) {
            uuu::upll_strncpy(
                reinterpret_cast<key_vtn_t *>(vtn_ckv->get_key())->vtn_name,
                reinterpret_cast<key_vtn_flowfilter_t *>
                (req->get_key())->vtn_key.vtn_name,
                (kMaxLenVtnName + 1));
          } else if (keytype == UNC_KT_VTN_FLOWFILTER_ENTRY) {
            uuu::upll_strncpy(
                reinterpret_cast<key_vtn *>(vtn_ckv->get_key())->vtn_name,
                reinterpret_cast<key_vtn_flowfilter_entry_t *>
                (req->get_key())->flowfilter_key.vtn_key.vtn_name,
                (kMaxLenVtnName + 1));
          } else if (keytype == UNC_KT_VTN_POLICINGMAP) {
            uuu::upll_strncpy(
                reinterpret_cast<key_vtn *>(vtn_ckv->get_key())->vtn_name,
                reinterpret_cast<key_vtn_t*>(req->get_key())->vtn_name,
                (kMaxLenVtnName + 1));
          } else if (keytype == UNC_KT_FLOWLIST_ENTRY) {
            uuu::upll_strncpy(
                reinterpret_cast<key_flowlist_t *>(vtn_ckv->get_key())->flowlist_name,
                reinterpret_cast<key_flowlist_entry_t *>
                (req->get_key())->flowlist_key.flowlist_name,
                (kMaxLenFlowListName + 1));
          } else if (keytype == UNC_KT_POLICING_PROFILE_ENTRY) {
            uuu::upll_strncpy(
                reinterpret_cast<key_policingprofile_t *>
                (vtn_ckv->get_key())->policingprofile_name,
                reinterpret_cast<key_policingprofile_entry *>
                (req->get_key())->policingprofile_key.policingprofile_name,
                (kMaxLenPolicingProfileName + 1));
          }

          result_code = mgr->UpdateConfigDB(vtn_ckv, UPLL_DT_IMPORT,
                                            UNC_OP_READ, dmi, MAINTBL);
          if (result_code != UPLL_RC_ERR_INSTANCE_EXISTS) {
            if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
              UPLL_LOG_DEBUG("UpdateConfigDB failed err code(%d)",
                             result_code);
              if (dal_cursor_handle)
                dmi->CloseCursor(dal_cursor_handle, true);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              DELETE_IF_NOT_NULL(vtn_ckv);
              return result_code;
            } else {
              if (keytype == UNC_KT_FLOWLIST_ENTRY) {
                FlowListEntryMoMgr *mgr = reinterpret_cast
                    <FlowListEntryMoMgr *>(const_cast<MoManager *>
                                           (GetMoManager(UNC_KT_FLOWLIST)));
                result_code = mgr->ChkFlowlistNameInRenameTbl(vtn_ckv,
                                                              UPLL_DT_IMPORT, dmi, ctrlr_id);
                if (result_code != UPLL_RC_SUCCESS &&
                    result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
                  UPLL_LOG_DEBUG("UpdateConfigDB failed err code(%d)",
                                 result_code);
                  if (dal_cursor_handle)
                    dmi->CloseCursor(dal_cursor_handle, true);
                  DELETE_IF_NOT_NULL(req);
                  DELETE_IF_NOT_NULL(nreq);
                  DELETE_IF_NOT_NULL(vtn_ckv);
                  return result_code;
                } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
                  UPLL_LOG_DEBUG("Deleted Record exists in IMPORT database(%d)",
                                 result_code);
                  DELETE_IF_NOT_NULL(vtn_ckv);
                  continue;
                }
              } else if (keytype == UNC_KT_POLICING_PROFILE_ENTRY) {
                PolicingProfileEntryMoMgr *mgr = reinterpret_cast
                    <PolicingProfileEntryMoMgr *>(const_cast<MoManager *>
                                                  (GetMoManager(UNC_KT_POLICING_PROFILE)));
                result_code = mgr->ChkProfileNameInRenameTbl(vtn_ckv,
                                                             UPLL_DT_IMPORT, dmi, ctrlr_id);
                if (result_code != UPLL_RC_SUCCESS &&
                    result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
                  UPLL_LOG_DEBUG("UpdateConfigDB failed err code(%d)",
                                 result_code);
                  if (dal_cursor_handle)
                    dmi->CloseCursor(dal_cursor_handle, true);
                  DELETE_IF_NOT_NULL(req);
                  DELETE_IF_NOT_NULL(nreq);
                  DELETE_IF_NOT_NULL(vtn_ckv);
                  return result_code;
                } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
                  UPLL_LOG_DEBUG("Deleted Record exists in IMPORT database(%d)",
                                 result_code);
                  DELETE_IF_NOT_NULL(vtn_ckv);
                  continue;
                }
              } else {
                UPLL_LOG_DEBUG("Deleted Record not exists in IMPORT database(%d)",
                               result_code);
                DELETE_IF_NOT_NULL(vtn_ckv);
                continue;
              }
            }
          }
          DELETE_IF_NOT_NULL(vtn_ckv);
        }

        result_code = DupConfigKeyVal(ck_main, req, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" DupConfigKeyVal failed err code(%d)", result_code);
          if (dal_cursor_handle)
            dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          return result_code;
        }
        result_code = mgr->GetChildConfigKey(ckv, NULL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed %d", result_code);
          if (dal_cursor_handle)
            dmi->CloseCursor(dal_cursor_handle, true);
          DELETE_IF_NOT_NULL(req);
          DELETE_IF_NOT_NULL(nreq);
          DELETE_IF_NOT_NULL(ck_main);
          return result_code;
        }
        if (keytype == UNC_KT_FLOWLIST) {
          uuu::upll_strncpy(
              reinterpret_cast<key_flowlist_t *>(ckv->get_key())->flowlist_name,
              reinterpret_cast<key_flowlist_t *>(ck_main->get_key())->flowlist_name,
              (kMaxLenFlowListName + 1));
        } else if (keytype == UNC_KT_FLOWLIST_ENTRY) {
          uuu::upll_strncpy(
              reinterpret_cast<key_flowlist_t *>(ckv->get_key())->flowlist_name,
              reinterpret_cast<key_flowlist_entry_t *>
              (ck_main->get_key())->flowlist_key.flowlist_name,
              (kMaxLenFlowListName + 1));
        } else if (keytype == UNC_KT_POLICING_PROFILE_ENTRY) {
          uuu::upll_strncpy(
              reinterpret_cast<key_policingprofile_t *>
              (ckv->get_key())->policingprofile_name,
              reinterpret_cast<key_policingprofile_entry *>
              (ck_main->get_key())->policingprofile_key.policingprofile_name,
              (kMaxLenPolicingProfileName + 1));
        } else if (keytype == UNC_KT_VTN_FLOWFILTER) {
          uuu::upll_strncpy(
              reinterpret_cast<key_vtn_t *>(ckv->get_key())->vtn_name,
              reinterpret_cast<key_vtn_flowfilter_t *>
              (ck_main->get_key())->vtn_key.vtn_name,
              (kMaxLenVtnName + 1));
        } else if (keytype == UNC_KT_VTN_FLOWFILTER_ENTRY) {
          uuu::upll_strncpy(
              reinterpret_cast<key_vtn *>(ckv->get_key())->vtn_name,
              reinterpret_cast<key_vtn_flowfilter_entry_t *>
              (ck_main->get_key())->flowfilter_key.vtn_key.vtn_name,
              (kMaxLenVtnName + 1));
        } else if (keytype == UNC_KT_VTN_POLICINGMAP) {
          uuu::upll_strncpy(
              reinterpret_cast<key_vtn *>(ckv->get_key())->vtn_name,
              reinterpret_cast<key_vtn_t*>(ck_main->get_key())->vtn_name,
              (kMaxLenVtnName + 1));
        }

        DbSubOp dbop = { kOpReadMultiple, kOpMatchNone , kOpInOutCtrlr};
        result_code = mgr->ReadConfigDB(ckv, UPLL_DT_CANDIDATE, UNC_OP_READ,
                                        dbop, dmi, CTRLRTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_TRACE("ReadConfigDB error %d ",result_code);
            if (dal_cursor_handle)
              dmi->CloseCursor(dal_cursor_handle, true);
            DELETE_IF_NOT_NULL(req);
            DELETE_IF_NOT_NULL(nreq);
            DELETE_IF_NOT_NULL(ckv);
            DELETE_IF_NOT_NULL(ck_main);
            return result_code;
          } else if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_DEBUG("Record not exists in ctrlr tbl");
            result_code = UPLL_RC_SUCCESS;
            DELETE_IF_NOT_NULL(ck_main);
            DELETE_IF_NOT_NULL(ckv);
            continue;
          }
        }

        ConfigKeyVal *ckval = ckv;
        while (ckval != NULL) {
          uint8_t *ctrlr = NULL;
          GET_USER_DATA_CTRLR(ckval, ctrlr);
          if (ctrlr) {
            UPLL_LOG_DEBUG("imported ctrlr (%s) candidate ctrlr (%s)",
                           ctrlr_id, ctrlr);
            if (strcmp(reinterpret_cast<char*>(ctrlr), ctrlr_id)) {
              UPLL_LOG_DEBUG("Object referred by other controller");
              if (dal_cursor_handle)
                dmi->CloseCursor(dal_cursor_handle, true);
              DELETE_IF_NOT_NULL(req);
              DELETE_IF_NOT_NULL(nreq);
              DELETE_IF_NOT_NULL(ckv);
              DELETE_IF_NOT_NULL(ck_main);
              return UPLL_RC_ERR_MERGE_CONFLICT;
            }
          }
          ckval = ckval->get_next_cfg_key_val();
        }
        DELETE_IF_NOT_NULL(ck_main);
        DELETE_IF_NOT_NULL(ckv);
      }
    }
    DELETE_IF_NOT_NULL(req);
    DELETE_IF_NOT_NULL(nreq);
    if (dal_cursor_handle)
      dmi->CloseCursor(dal_cursor_handle, true);
  }

  UPLL_LOG_DEBUG("MergeValidate result code (%d)", result_code);
  return result_code;
}

upll_rc_t MoMgrImpl::ValidateIpAddress(ConfigKeyVal* ikey,
                                       upll_keytype_datatype_t dt_type,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t     result_code = UPLL_RC_SUCCESS;
  VbrMoMgr      *vbr_mgr    = NULL;
  VrtIfMoMgr    *vrtif_mgr  = NULL;
  ConfigKeyVal  *vrtif_ckv  = NULL;
  ConfigKeyVal  *vbr_ckv    = NULL;
  uint8_t       *input_vbr_name = NULL;
  uint8_t       *input_vrt_name = NULL;
  uint8_t       *input_vrtif_name = NULL;
  struct in_addr  ip_addr;
  ip_addr.s_addr = 0;

  if (!GetVal(ikey)) {
    UPLL_LOG_DEBUG("Semantic check not required");
    return UPLL_RC_SUCCESS;
  }

  /* Saves received ikey vtn_name,vnode_name,ip_addr and prefix_len*/
  if (ikey->get_key_type() == UNC_KT_VBRIDGE) {
    key_vbr_t *vbrkey = reinterpret_cast<key_vbr_t*>(ikey->get_key());
    val_vbr_t *vbrval = reinterpret_cast<val_vbr_t*>(GetVal(ikey));
    if ((vbrval->valid[UPLL_IDX_HOST_ADDR_VBR] != UNC_VF_VALID) &&
        (vbrval->valid[UPLL_IDX_HOST_ADDR_PREFIXLEN_VBR] != UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("Semantic check not required");
      return UPLL_RC_SUCCESS;
    }
    input_vbr_name = vbrkey->vbridge_name;
    ip_addr.s_addr = vbrval->host_addr.s_addr;
  } else if (ikey->get_key_type() == UNC_KT_VRT_IF) {
    key_vrt_if_t *vrtifkey = reinterpret_cast<key_vrt_if_t*>(
                                ikey->get_key());
    val_vrt_if_t *vrtifval = reinterpret_cast<val_vrt_if_t*>(GetVal(ikey));
    if ((vrtifval->valid[UPLL_IDX_IP_ADDR_VI] != UNC_VF_VALID) &&
        (vrtifval->valid[UPLL_IDX_PREFIXLEN_VI] != UNC_VF_VALID)) {
      UPLL_LOG_DEBUG("Semantic check not required");
      return UPLL_RC_SUCCESS;
    }

    input_vrt_name   = vrtifkey->vrt_key.vrouter_name;
    input_vrtif_name = vrtifkey->if_name;
    ip_addr.s_addr   = vrtifval->ip_addr.s_addr;
  }

  vrtif_mgr = reinterpret_cast<VrtIfMoMgr *>(
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
   * vrouter interface. If it is configured for another interface means Semantic
   * error else Checks in to vbridge table */
  DbSubOp dbop = { kOpReadSingle, kOpMatchNone, kOpInOutNone };
  result_code = vrtif_mgr->ReadConfigDB(vrtif_ckv, dt_type, UNC_OP_READ, dbop,
                                        dmi, MAINTBL);
  if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    if (result_code == UPLL_RC_SUCCESS) {
      if (ikey->get_key_type() == UNC_KT_VBRIDGE) {
        UPLL_LOG_DEBUG("Same Ip Address is already configured for another"
                       "vbridge");
        DELETE_IF_NOT_NULL(vrtif_ckv);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      } else if (ikey->get_key_type() == UNC_KT_VRT_IF) {
        if (strcmp(reinterpret_cast<char*>(vrtif_key->vrt_key.vrouter_name),
              reinterpret_cast<char*>(input_vrt_name))) {
          UPLL_LOG_DEBUG("Same Ip Address is already configured for another"
                         " vrouter ");
          DELETE_IF_NOT_NULL(vrtif_ckv);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        } else if (strcmp(reinterpret_cast<char*>(vrtif_key->if_name),
              reinterpret_cast<char*>(input_vrtif_name))) {
          UPLL_LOG_DEBUG("Same Ip Address is already configured for another"
                         " vrouter interface");
          DELETE_IF_NOT_NULL(vrtif_ckv);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
    }
    DELETE_IF_NOT_NULL(vrtif_ckv);
    return result_code;
  }
  DELETE_IF_NOT_NULL(vrtif_ckv);

  vbr_mgr = reinterpret_cast<VbrMoMgr *>(
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
   * vrouter interface. If it is configured for another interface means Semantic
   * error else Checks in to vbridge table */
  result_code = vbr_mgr->ReadConfigDB(vbr_ckv, dt_type, UNC_OP_READ, dbop, dmi,
                                      MAINTBL);
  if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    if (result_code == UPLL_RC_SUCCESS) {
      if (ikey->get_key_type() == UNC_KT_VRT_IF) {
        UPLL_LOG_DEBUG("Same Ip Address is already configured for another"
                       " vbridge vrouter interface");
        DELETE_IF_NOT_NULL(vbr_ckv);
        return UPLL_RC_ERR_CFG_SEMANTIC;
      } else if (ikey->get_key_type() == UNC_KT_VBRIDGE) {
        if (strcmp(reinterpret_cast<char*>(vbr_key->vbridge_name),
              reinterpret_cast<char*>(input_vbr_name))) {
          UPLL_LOG_DEBUG("Same Ip Address is already configured for another"
              " vbridge ");
          DELETE_IF_NOT_NULL(vbr_ckv);
          return UPLL_RC_ERR_CFG_SEMANTIC;
        }
      }
    }
    DELETE_IF_NOT_NULL(vbr_ckv);
    return result_code;
  }
  DELETE_IF_NOT_NULL(vbr_ckv);
  return UPLL_RC_SUCCESS;
}
/* U14 Requirement Change */

/*
 *  This function used to chekcs the given node is
 *  unique or not
 */
upll_rc_t MoMgrImpl::PartialImport_VnodeChecks(ConfigKeyVal *ikey,
                                 upll_keytype_datatype_t dt_type,
                                 const char *ctrlr_id,
                                 DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  if (!ikey || !(ikey->get_key()))
    return UPLL_RC_ERR_GENERIC;
  unc_key_type_t nodes[] = {UNC_KT_VBRIDGE, UNC_KT_VROUTER, UNC_KT_VTERMINAL,
                            UNC_KT_VUNKNOWN, UNC_KT_VTEP, UNC_KT_VTUNNEL};
  int nop = sizeof(nodes)/ sizeof(nodes[0]);
  ConfigKeyVal *ck_vnode = NULL;
  UPLL_LOG_TRACE("ikey keytype %d", ikey->get_key_type());
  DbSubOp dbop = { kOpReadMultiple, kOpMatchNone, kOpInOutCtrlr };
  for (int indx = 0; indx < nop; indx++) {
    MoMgrImpl *mgr = reinterpret_cast<MoMgrImpl *>(
        const_cast<MoManager *>(GetMoManager(nodes[indx])));
    if (!mgr) {
      UPLL_LOG_TRACE("Invalid mgr");
      continue;
    }
    result_code = mgr->CreateVnodeConfigKey(ikey, ck_vnode);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("CreateVnodeConfigKey failed - %d", result_code);
      return result_code;
    }
    result_code = mgr->ReadConfigDB(ck_vnode, dt_type, UNC_OP_READ,
                                    dbop, dmi, MAINTBL);
    if (result_code != UPLL_RC_SUCCESS &&
        result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UPLL_LOG_TRACE("Existence check in keytype %d result_code  %d",
        ck_vnode->get_key_type(), result_code);
      delete ck_vnode;
      return result_code;
    }
    uint8_t *ctrlr_name = NULL;
    if (UPLL_RC_SUCCESS == result_code) {
      GET_USER_DATA_CTRLR(ck_vnode, ctrlr_name);
      if (strcmp(ctrlr_id, (const char*)ctrlr_name)) {
        UPLL_LOG_INFO("controller name is different");
        delete ck_vnode;
        return UPLL_RC_ERR_MERGE_CONFLICT;
      }
      if (ikey->get_key_type() != ck_vnode->get_key_type()) {
        UPLL_LOG_INFO("vnode already exists in another vnode tbl");
      //  result_code = UPLL_RC_ERR_CFG_SEMANTIC;
      }
      delete ck_vnode;
      return result_code;
    }
    if (ck_vnode) {
      delete ck_vnode;
      ck_vnode = NULL;
    }
  }
  VbrIfMoMgr *vbrifmgr = reinterpret_cast<VbrIfMoMgr *>
      (const_cast<MoManager *>(GetMoManager(UNC_KT_VBR_IF)));
  if (vbrifmgr == NULL) {
    UPLL_LOG_DEBUG("Invalid momgr object");
    return UPLL_RC_ERR_GENERIC;
  }
  ConfigKeyVal *vbrif_key_val = NULL;
  controller_domain_t ctr_dom;
  memset(&ctr_dom, 0, sizeof(controller_domain_t));
  result_code = GetControllerDomainId(ikey, &ctr_dom);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetControllerDomainId failed");
    return result_code;
  }

  if (!ctr_dom.ctrlr || !ctr_dom.domain) {
    GET_USER_DATA_CTRLR_DOMAIN(ikey, ctr_dom);
  }

  result_code = vbrifmgr->GetVbrIfFromVExternal(
      reinterpret_cast<key_vnode_t *>
      (ikey->get_key())->vtn_key.vtn_name,
      reinterpret_cast<key_vnode_t *>(ikey->get_key())->vnode_name,
      vbrif_key_val, dmi, ctr_dom, dt_type);
  if (UPLL_RC_SUCCESS == result_code)
    result_code = UPLL_RC_ERR_CFG_SEMANTIC;
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ? UPLL_RC_SUCCESS :
      result_code;
  DELETE_IF_NOT_NULL(vbrif_key_val);
  return result_code;
}

upll_rc_t MoMgrImpl::PurgeCandidate(unc_key_type_t keytype,
                                     const char *ctrlr_id,
                                     DalDmlIntf *dmi) {

  UPLL_FUNC_TRACE;
  if (OVERLAY_KT(keytype) || UNKNOWN_KT(keytype))
    return UPLL_RC_SUCCESS;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  UPLL_LOG_TRACE("MergeImportToCandidate ");
  unc_keytype_operation_t op = UNC_OP_DELETE;
  ConfigKeyVal *temp_ckv = NULL;
  result_code = GetChildConfigKey(temp_ckv, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    return result_code;
  }
  SET_USER_DATA_CTRLR(temp_ckv, ctrlr_id);
  for (int tbl = MAINTBL; tbl < ntable; tbl++) {
    if (tbl == MAINTBL && (UNC_KT_VTN == keytype ||
                           UNC_KT_FLOWLIST == keytype ||
                           UNC_KT_POLICING_PROFILE == keytype ||
                           UNC_KT_VTN_FLOWFILTER  == keytype ||
                           UNC_KT_VTN_FLOWFILTER_ENTRY == keytype ||
                           keytype == UNC_KT_VTN_POLICINGMAP ||
                           keytype == UNC_KT_POLICING_PROFILE_ENTRY ||
                           keytype == UNC_KT_FLOWLIST_ENTRY)) {
      continue;
    }
    uudst::kDalTableIndex tbl_index;
    tbl_index = GetTable((MoMgrTables)tbl, UPLL_DT_IMPORT);
    /* skipping VTN Main Table */
    if (tbl_index < uudst::kDalNumTables && tbl_index != 0) {
      UPLL_LOG_TRACE("Merging the Keytype %d, Table %d, Table index %d ",
                     keytype, tbl, tbl_index);
      if (table[tbl]->get_key_type() != keytype) {
        delete temp_ckv;
        return UPLL_RC_ERR_GENERIC;
      }
      DalBindInfo dal_bind_info(tbl_index);
      //dal_bind_info = new DalBindInfo(tbl_index);
      DbSubOp dbop = { kOpReadMultiple, kOpMatchCtrlr, kOpInOutNone};
      result_code = BindAttr(&dal_bind_info, temp_ckv, UNC_OP_DELETE,
                             UPLL_DT_CANDIDATE, dbop,(MoMgrTables)(tbl));
      if (UPLL_RC_SUCCESS != result_code) {
        delete temp_ckv;
        return result_code;
      }
      result_code = DalToUpllResCode(dmi->CopyModifiedRecords(
              UPLL_DT_CANDIDATE,
              UPLL_DT_IMPORT, tbl_index,
              &dal_bind_info, op));
      UPLL_LOG_TRACE("%d Table is completed", tbl);
      if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code)
        result_code = UPLL_RC_SUCCESS;
    }
    if (UPLL_RC_SUCCESS != result_code) {
//      delete dal_bind_info;
        delete temp_ckv;
        return result_code;
    }
//  delete dal_bind_info;
  }
  delete temp_ckv;
  UPLL_LOG_TRACE("MergeValidate with dalbind info");
  // convert dal_result to result_code
#if 0
  if (UPLL_RC_SUCCESS == result_code &&
     (keytype == UNC_KT_VTN ||
      keytype == UNC_KT_FLOWLIST ||
      keytype == UNC_KT_POLICING_PROFILE ||
      keytype == UNC_KT_VTN_FLOWFILTER ||
      keytype == UNC_KT_VTN_FLOWFILTER_ENTRY ||
      keytype == UNC_KT_VTN_POLICINGMAP ||
      keytype == UNC_KT_POLICING_PROFILE_ENTRY ||
      keytype == UNC_KT_FLOWLIST_ENTRY)) {
    result_code = DeleteGlobalConfigInCandidate(keytype, dmi);
  }
#endif
  return result_code;
}

upll_rc_t MoMgrImpl::GlobalAuditUpdateController(unc_key_type_t keytype,
    const char *ctrlr_id,
    uint32_t session_id,
    uint32_t config_id,
    uuc::UpdateCtrlrPhase phase,
    DalDmlIntf *dmi,
    ConfigKeyVal **err_ckv,
    KTxCtrlrAffectedState *ctrlr_affected) {

  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result = uud::kDalRcSuccess;
  MoMgrTables tbl  = MAINTBL;
  controller_domain_t ctrlr_dom;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  ConfigKeyVal  *ckv_running = NULL;
  ConfigKeyVal  *ckv_audit = NULL;
  ConfigKeyVal  *ckv_drvr = NULL;
  ConfigKeyVal  *resp = NULL;
  ConfigKeyVal  *ctrlr_ckv = NULL;
  ConfigKeyVal  *failed_ckv = NULL;
  DalCursor *cursor = NULL;
  IpcResponse ipc_response;
  bool invalid_attr = false;
  // Specifies true for audit update transaction
  // else false default
  const bool audit_update_phase = true;
  uint8_t *in_ctrlr = reinterpret_cast<uint8_t *>(const_cast<char *>(ctrlr_id));
  // Decides whether to retrieve from controller table or main table
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
    ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
     ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));
  if (phase == uuc::kUpllUcpDelete2)
    return result_code;
  if (op == UNC_OP_INVALID) {
    UPLL_LOG_INFO("Invalid operation received-%d", op);
    // Not a valid operation
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  /* retreives the delta of running and audit configuration */
  UPLL_LOG_TRACE("Operation is %d", op);
  //  Op - Create and Update - Diff in Ctrlr Tbl
  //  Op - Update - Diff in MainTbl
  if (op == UNC_OP_UPDATE) {
    tbl = MAINTBL;
  } else {
    tbl = CTRLRTBL;
  }
  result_code = DiffConfigDB(UPLL_DT_RUNNING, UPLL_DT_AUDIT, op,
      ckv_running, ckv_audit,
      &cursor, dmi, in_ctrlr, tbl, true);

  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DiffConfigDB failed - %d", result_code);
    if (cursor)
      dmi->CloseCursor(cursor, true);
    DELETE_IF_NOT_NULL(ckv_running);
    DELETE_IF_NOT_NULL(ckv_audit);
    return result_code;
  }

  while (uud::kDalRcSuccess == (db_result = dmi->GetNextRecord(cursor))) {
    if (op == UNC_OP_CREATE || op == UNC_OP_DELETE) {
      if (phase != uuc::kUpllUcpDelete) {
        uint8_t *db_ctrlr = NULL;
        GET_USER_DATA_CTRLR(ckv_running, db_ctrlr);
        if ((!db_ctrlr) || (db_ctrlr && strncmp(reinterpret_cast<const char *>(db_ctrlr),
                reinterpret_cast<const char *>(ctrlr_id),
                strlen(reinterpret_cast<const char *>(ctrlr_id)) + 1))) {
          continue;
        }
      }
      result_code =  GetDiffRecord(ckv_running, ckv_audit, phase, tbl,
          ckv_drvr, dmi, invalid_attr, audit_update_phase);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("GetDiffRecord failed err code is %d", result_code);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
      if (invalid_attr) {
        DELETE_IF_NOT_NULL(ckv_drvr);
        // Assuming that the diff found only in ConfigStatus
        // Setting the value as OnlyCSDiff in the out parameter ctrlr_affected
        // The value Configdiff should be given more priority than the value
        // only cs.
        // So, if the out parameter ctrlr_affected has already value as configdiff
        // then dont change the value
        if (*ctrlr_affected != uuc::kCtrlrAffectedConfigDiff) {
          UPLL_LOG_INFO("Setting the ctrlr_affected to OnlyCSDiff for KT %u",
              keytype);
          *ctrlr_affected = uuc::kCtrlrAffectedOnlyCSDiff;
        }
        continue;
      }
      // Case1: commit(del) - Check in AUDIT since no info exists in RUNNING
      // Case2: Commit(Cr/upd) - Check in RUNNING always
      upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
        UPLL_DT_AUDIT : UPLL_DT_RUNNING;
      // not_send_to_drv is to decide whether the configuration needs
      // to be sent to controller or not
      bool not_send_to_drv = false;
      // Perform semantic validation and corresponding
      // vexternal/vexternalif conversion on key type specific,
      // before sending to driver.
      result_code = AdaptValToDriver(ckv_drvr, NULL, op,
          dt_type, keytype, dmi, not_send_to_drv, audit_update_phase);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("AdaptValToDriver failed %d", result_code);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
      if (not_send_to_drv) {
        UPLL_LOG_TRACE("%s Configuration is not sent to controller",
            (ckv_drvr->ToStr()).c_str());
        DELETE_IF_NOT_NULL(ckv_drvr);
        continue;
      }
      // Get controller & domain-id from ConfigKeyVal 'ckv_drvr'
      GET_USER_DATA_CTRLR_DOMAIN(ckv_drvr, ctrlr_dom);
      result_code = DupConfigKeyVal(resp, ckv_drvr, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" DupConfigKeyVal failed err code(%d)",
            result_code);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
      result_code = GetRenamedControllerKey(ckv_drvr, UPLL_DT_RUNNING,
          dmi, &ctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" GetRenamedControllerKey failed err code(%d)",
            result_code);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(resp);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
      if (ctrlr_dom.ctrlr != NULL) {
        bool domain = false;
        KEYTYPE_WITHOUT_DOMAIN(keytype, domain);
        if (!domain) {
          if (NULL == ctrlr_dom.domain) {
            UPLL_LOG_INFO(" domain is NULL");
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            DELETE_IF_NOT_NULL(ckv_drvr);
            DELETE_IF_NOT_NULL(resp);
            dmi->CloseCursor(cursor, true);
            return UPLL_RC_ERR_GENERIC;
          }
        }
      } else {
        UPLL_LOG_DEBUG("Controller Id is NULL");
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(resp);
        dmi->CloseCursor(cursor, true);
        return UPLL_RC_ERR_GENERIC;
      }
      memset(&ipc_response, 0, sizeof(IpcResponse));
      IpcRequest ipc_req;
      memset(&ipc_req, 0, sizeof(IpcRequest));
      ipc_req.header.clnt_sess_id = session_id;
      ipc_req.header.config_id = config_id;
      ipc_req.header.operation = op;
      ipc_req.header.datatype = UPLL_DT_CANDIDATE;
      ipc_req.ckv_data = ckv_drvr;
      // To populate IPC request.
      if (!IpcUtil::SendReqToDriver((const char *)ctrlr_dom.ctrlr, reinterpret_cast<char *>
            (ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
            PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_response)) {
        UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
            ckv_drvr->get_key_type(), reinterpret_cast<char *>(ctrlr_dom.ctrlr));
        DELETE_IF_NOT_NULL(resp);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        dmi->CloseCursor(cursor, true);
        return ipc_response.header.result_code;
      }
      if (ipc_response.header.result_code != UPLL_RC_SUCCESS) {
        if (phase == uuc::kUpllUcpCreate) {
          result_code = DupConfigKeyVal(failed_ckv, ckv_running, CTRLRTBL);
          if (UPLL_RC_SUCCESS != result_code) {
            DELETE_IF_NOT_NULL(resp);
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            DELETE_IF_NOT_NULL(ckv_drvr);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
        }
      }
    } else {
      result_code = DupConfigKeyVal(ckv_drvr, ckv_running, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("DupConfigKeyVal failed %d", result_code);
        dmi->CloseCursor(cursor, true);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        return result_code;
      }
      if (NULL != GetVal(ckv_drvr) &&
          NULL != GetVal(ckv_audit)) {
        void *val1 = GetVal(ckv_drvr);
        invalid_attr = FilterAttributes(val1, GetVal(ckv_audit),
            false, UNC_OP_UPDATE);
        if (invalid_attr) {
          if (*ctrlr_affected != uuc::kCtrlrAffectedConfigDiff) {
            UPLL_LOG_INFO("Setting the ctrlr_affected to OnlyCSDiff for KT %u",
                keytype);
            *ctrlr_affected = uuc::kCtrlrAffectedOnlyCSDiff;
          }
          DELETE_IF_NOT_NULL(ckv_drvr);
          continue;
        }
      } else {
        UPLL_LOG_INFO("Val struct is NULL");
        dmi->CloseCursor(cursor, true);
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        return UPLL_RC_ERR_GENERIC;
      }
      bool not_send_to_drv = false;
      result_code = AdaptValToDriver(ckv_drvr, NULL, UNC_OP_UPDATE,
          UPLL_DT_RUNNING, keytype, dmi, not_send_to_drv,
          audit_update_phase);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("AdaptValToDriver failed %d", result_code);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
      if (not_send_to_drv) {
        UPLL_LOG_TRACE("%s Configuration is not sent to controller",
            (ckv_drvr->ToStr()).c_str());
        DELETE_IF_NOT_NULL(ckv_drvr);
        continue;
      }
      // Set the ctrlr name to ckv_drvr
      SET_USER_DATA_CTRLR(ckv_drvr, in_ctrlr);
      result_code = GetDomainsForController(ckv_drvr, ctrlr_ckv, dmi);
      if (UPLL_RC_SUCCESS != result_code &&
          UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
        UPLL_LOG_INFO("GetDomainsForController failed %d", result_code);
        dmi->CloseCursor(cursor, true);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(ctrlr_ckv);
        return result_code;
      }
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(ctrlr_ckv);
        continue;
      }
      result_code = DupConfigKeyVal(resp, ckv_drvr, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" DupConfigKeyVal failed err code(%d)",
            result_code);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        DELETE_IF_NOT_NULL(ctrlr_ckv);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
      ConfigKeyVal *temp_ctrlr_ckv = ctrlr_ckv;
      while (NULL != temp_ctrlr_ckv ) {
        ConfigKeyVal *temp_ckv_drvr = NULL;
        result_code = DupConfigKeyVal(temp_ckv_drvr, ckv_drvr, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG(" DupConfigKeyVal failed err code(%d)",
              result_code);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(ctrlr_ckv);
          DELETE_IF_NOT_NULL(resp);
          dmi->CloseCursor(cursor, true);
          return result_code;
        }
        GET_USER_DATA_CTRLR_DOMAIN(temp_ctrlr_ckv, ctrlr_dom);
        SET_USER_DATA_CTRLR_DOMAIN(temp_ckv_drvr, ctrlr_dom);
        ConfigVal *next_val = (ckv_audit->get_cfg_val())->DupVal();
        temp_ckv_drvr->AppendCfgVal(next_val);
        result_code = GetRenamedControllerKey(temp_ckv_drvr, UPLL_DT_RUNNING,
            dmi, &ctrlr_dom);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_INFO(" GetRenamedControllerKey failed err code(%d)",
              result_code);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(resp);
          DELETE_IF_NOT_NULL(ctrlr_ckv);
          DELETE_IF_NOT_NULL(temp_ckv_drvr);
          dmi->CloseCursor(cursor, true);
          return result_code;
        }
        memset(&ipc_response, 0, sizeof(IpcResponse));
        IpcRequest ipc_req;
        memset(&ipc_req, 0, sizeof(IpcRequest));
        ipc_req.header.clnt_sess_id = session_id;
        ipc_req.header.config_id = config_id;
        ipc_req.header.operation = op;
        ipc_req.header.datatype = UPLL_DT_CANDIDATE;
        ipc_req.ckv_data = temp_ckv_drvr;
        // To populate IPC request.
        if (!IpcUtil::SendReqToDriver((const char *)ctrlr_dom.ctrlr, reinterpret_cast<char *>
              (ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
              PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_response)) {
          UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
              ckv_drvr->get_key_type(), reinterpret_cast<char *>(ctrlr_dom.ctrlr));
          DELETE_IF_NOT_NULL(resp);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          DELETE_IF_NOT_NULL(temp_ckv_drvr);
          DELETE_IF_NOT_NULL(ctrlr_ckv);
          DELETE_IF_NOT_NULL(ipc_response.ckv_data);
          dmi->CloseCursor(cursor, true);
          return ipc_response.header.result_code;
        }
        if (UPLL_RC_SUCCESS != ipc_response.header.result_code) {
          UPLL_LOG_DEBUG("SendReqToDriver failed during update");
          result_code = DupConfigKeyVal(failed_ckv, temp_ctrlr_ckv, CTRLRTBL);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed %d", result_code);
            DELETE_IF_NOT_NULL(ipc_response.ckv_data);
            DELETE_IF_NOT_NULL(resp);
            DELETE_IF_NOT_NULL(ckv_running);
            DELETE_IF_NOT_NULL(ckv_audit);
            DELETE_IF_NOT_NULL(ckv_drvr);
            DELETE_IF_NOT_NULL(temp_ckv_drvr);
            DELETE_IF_NOT_NULL(ctrlr_ckv);
            dmi->CloseCursor(cursor, true);
            return result_code;
          }
          break;
        }
        DELETE_IF_NOT_NULL(temp_ckv_drvr);
        temp_ctrlr_ckv = temp_ctrlr_ckv->get_next_cfg_key_val();
      }  // End of Update Ctrlr_dom while loop
      DELETE_IF_NOT_NULL(ctrlr_ckv);
    }  // End of if-else
    if  (ipc_response.header.result_code != UPLL_RC_SUCCESS) {
      *err_ckv = resp;
      if (phase == uuc::kUpllUcpDelete) {
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        return ipc_response.header.result_code;
      }
      result_code = UpdateAuditConfigStatus(UNC_CS_INVALID,
          phase, failed_ckv, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("UpdateAuditConfigStatus failed %d", result_code);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        DELETE_IF_NOT_NULL(failed_ckv);
        DELETE_IF_NOT_NULL(resp);
        *err_ckv = NULL;
        return result_code;
      }
      result_code = UpdateConfigDB(failed_ckv, UPLL_DT_RUNNING, UNC_OP_UPDATE,
          dmi, CTRLRTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("UpdateConfigDB failed for ipc response ckv err_code %d",
            result_code);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        DELETE_IF_NOT_NULL(failed_ckv);
        DELETE_IF_NOT_NULL(resp);
        *err_ckv = NULL;
        return result_code;
      }
      DELETE_IF_NOT_NULL(failed_ckv);
      result_code = SetConsolidatedStatus(resp, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("SetConsolidatedStatus failed for ipc response ckv err_code %d",
            result_code);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        DELETE_IF_NOT_NULL(resp);
        *err_ckv = NULL;
        return result_code;
      }
      DELETE_IF_NOT_NULL(ipc_response.ckv_data);
      DELETE_IF_NOT_NULL(ckv_running);
      DELETE_IF_NOT_NULL(ckv_audit);
      DELETE_IF_NOT_NULL(ckv_drvr);
      dmi->CloseCursor(cursor, true);
      return ipc_response.header.result_code;
    } // End of Driver Error Handling
    DELETE_IF_NOT_NULL(ckv_drvr);
    DELETE_IF_NOT_NULL(ipc_response.ckv_data);
    if (*ctrlr_affected == uuc::kCtrlrAffectedOnlyCSDiff) {
      UPLL_LOG_INFO("Reset ctrlr state from OnlyCSDiff to ConfigDiff  KT %u",
          keytype);
    }
    UPLL_LOG_DEBUG("Setting the ctrlr_affected to ConfigDiff, KT %u", keytype);
    *ctrlr_affected = uuc::kCtrlrAffectedConfigDiff;
    DELETE_IF_NOT_NULL(resp);
  }  // End of main while loop
  if (cursor)
    dmi->CloseCursor(cursor, true);
  if (uud::kDalRcSuccess != db_result) {
    UPLL_LOG_DEBUG("GetNextRecord from database failed  - %d", db_result);
    result_code =  DalToUpllResCode(db_result);
  }
  DELETE_IF_NOT_NULL(ckv_running);
  DELETE_IF_NOT_NULL(ckv_audit);
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
    ? UPLL_RC_SUCCESS : result_code;
  return result_code;
}

upll_rc_t MoMgrImpl::GetRenamedUncKeyWoRedirection(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type, DalDmlIntf *dmi, uint8_t *ctrlr_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_TRACE("%s GetRenamedUncKey vbrff_entry start",
                  ikey->ToStrAll().c_str());
  ConfigKeyVal *unc_key = NULL;
  key_vbr_flowfilter_entry_t *key = NULL;
  key_vbr_if_flowfilter_entry_t *key1 = NULL;
  key_vrt_if_flowfilter_entry_t *key2 = NULL;
  key_vterm_if_flowfilter_entry_t *key3 = NULL;

  uint8_t rename = 0;
  if ((NULL == ikey) || (ctrlr_id == NULL) || (NULL == dmi)) {
    UPLL_LOG_DEBUG("ikey/ctrlr_id dmi NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  MoMgrImpl *mgr = NULL;

  if (ikey->get_key_type() == UNC_KT_VBR_FLOWFILTER_ENTRY ||
      ikey->get_key_type() == UNC_KT_VBRIF_FLOWFILTER_ENTRY) {
    mgr = reinterpret_cast<MoMgrImpl *>
          (const_cast<MoManager *> (GetMoManager(UNC_KT_VBRIDGE)));
  } else if (ikey->get_key_type() == UNC_KT_VRTIF_FLOWFILTER_ENTRY) {
    mgr = static_cast<MoMgrImpl*>
    ((const_cast<MoManager*> (GetMoManager(UNC_KT_VROUTER))));
  } else if (ikey->get_key_type() == UNC_KT_VTERMIF_FLOWFILTER_ENTRY) {
    mgr = static_cast<MoMgrImpl*>
    ((const_cast<MoManager*>(GetMoManager(UNC_KT_VTERMINAL))));
  }

  if (!mgr) {
    UPLL_LOG_ERROR("mgr failed");
    return UPLL_RC_ERR_GENERIC;
  }

  val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode*>
      (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
  if (!rename_val) {
    UPLL_LOG_TRACE("rename_val NULL");
    return UPLL_RC_ERR_GENERIC;
  }

  switch (ikey->get_key_type()) {
  case UNC_KT_VBR_FLOWFILTER_ENTRY:
     UPLL_LOG_DEBUG("vbr_ff_entry");
     key =
     reinterpret_cast<key_vbr_flowfilter_entry_t *>(ikey->get_key());

     uuu::upll_strncpy(rename_val->ctrlr_vtn_name,
                        key->flowfilter_key.vbr_key.vtn_key.vtn_name,
                        kMaxLenVtnName + 1);
     rename_val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
     uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
                        key->flowfilter_key.vbr_key.vbridge_name,
                        kMaxLenVnodeName + 1);
     rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
  break;
  case UNC_KT_VBRIF_FLOWFILTER_ENTRY:
     UPLL_LOG_DEBUG("vbr_if_ff_entry");
     key1 =
     reinterpret_cast<key_vbr_if_flowfilter_entry_t *>(ikey->get_key());
     uuu::upll_strncpy(rename_val->ctrlr_vtn_name,
          key1->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
     rename_val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
     uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
          key1->flowfilter_key.if_key.vbr_key.vbridge_name,
          kMaxLenVnodeName + 1);
     rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
  break;
  case UNC_KT_VRTIF_FLOWFILTER_ENTRY:
     UPLL_LOG_DEBUG("vrt_if_ff_entry");
     key2 =
     reinterpret_cast<key_vrt_if_flowfilter_entry_t *>(ikey->get_key());
      uuu::upll_strncpy(rename_val->ctrlr_vtn_name,
          key2->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      rename_val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
      uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
          key2->flowfilter_key.if_key.vrt_key.vrouter_name,
          kMaxLenVnodeName + 1);
     rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
  break;
  case UNC_KT_VTERMIF_FLOWFILTER_ENTRY:
     UPLL_LOG_DEBUG("vterm_if_ff_entry");
      key3 =
      reinterpret_cast<key_vterm_if_flowfilter_entry_t *>(ikey->get_key());
      uuu::upll_strncpy(rename_val->ctrlr_vtn_name,
          key3->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
          kMaxLenVtnName + 1);
      rename_val->valid[UPLL_CTRLR_VTN_NAME_VALID] = UNC_VF_VALID;
      uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
          key3->flowfilter_key.if_key.vterm_key.vterminal_name,
          kMaxLenVnodeName + 1);
      rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;
  break;
  default:
      UPLL_LOG_DEBUG("Invalid keytype");
      return UPLL_RC_ERR_GENERIC;
  break;
  }

  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("GetChildConfigKey returned error");
    free(rename_val);
    mgr = NULL;
    return result_code;
  }

  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_val);
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ,
      dbop, dmi, RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
    UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
    DELETE_IF_NOT_NULL(unc_key);
    mgr = NULL;
    return result_code;
  }

  if (result_code == UPLL_RC_SUCCESS) {
    if (ikey->get_key_type() == UNC_KT_VBR_FLOWFILTER_ENTRY) {
     UPLL_LOG_DEBUG("vbr_ff_entry1");
    key_vbr_flowfilter_entry_t *vbr_key =
        reinterpret_cast<key_vbr_flowfilter_entry_t *> (unc_key->get_key());
    if (strcmp(reinterpret_cast<char *>(key->flowfilter_key
               .vbr_key.vtn_key.vtn_name),
               reinterpret_cast<const char *>(vbr_key
               ->flowfilter_key.vbr_key.vtn_key.vtn_name))) {
       uuu::upll_strncpy(key->flowfilter_key.vbr_key.vtn_key.vtn_name,
            vbr_key->flowfilter_key.vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName + 1));
       rename |= VTN_RENAME;
    }
    if (strcmp(reinterpret_cast<char *>(key->flowfilter_key
               .vbr_key.vbridge_name),
               reinterpret_cast<const char *>(vbr_key->
               flowfilter_key.vbr_key.vbridge_name))) {
       uuu::upll_strncpy(key->flowfilter_key.vbr_key.vbridge_name,
              vbr_key->flowfilter_key.vbr_key.vbridge_name,
              (kMaxLenVnodeName + 1));
      rename |= VN_RENAME;
    }
   } else if (ikey->get_key_type() == UNC_KT_VBRIF_FLOWFILTER_ENTRY) {
    UPLL_LOG_DEBUG("vbr_if_ff_entry1");
    key_vbr_if_flowfilter_entry_t *vbr_if_key = reinterpret_cast
      <key_vbr_if_flowfilter_entry_t *>(unc_key->get_key());

    if (strcmp(reinterpret_cast<char *>(key1->flowfilter_key.if_key
               .vbr_key.vtn_key.vtn_name),
               reinterpret_cast<const char *>(vbr_if_key->
               flowfilter_key.if_key.vbr_key.vtn_key.vtn_name))) {
      uuu::upll_strncpy(
      key1->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
      vbr_if_key->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
      kMaxLenVtnName + 1);
      rename |= VTN_RENAME;
    }
    if (strcmp(reinterpret_cast<char *>(key1->flowfilter_key.if_key
               .vbr_key.vbridge_name),
               reinterpret_cast<const char *>(vbr_if_key->
               flowfilter_key.if_key.vbr_key.vbridge_name))) {
      uuu::upll_strncpy(
      key1->flowfilter_key.if_key.vbr_key.vbridge_name,
      vbr_if_key->flowfilter_key.if_key.vbr_key.vbridge_name,
      kMaxLenVnodeName + 1);
      rename |= VN_RENAME;
    }
   } else if (ikey->get_key_type() == UNC_KT_VRTIF_FLOWFILTER_ENTRY) {
    UPLL_LOG_DEBUG("vrt_if_ff_entry1");
    key_vrt_if_flowfilter_entry_t *vrt_if_key = reinterpret_cast
    <key_vrt_if_flowfilter_entry_t *> (unc_key->get_key());
    if (strcmp(reinterpret_cast<char *>(key2->
               flowfilter_key.if_key.vrt_key.vtn_key.vtn_name),
               reinterpret_cast<const char *>(vrt_if_key->
               flowfilter_key.if_key.vrt_key.vtn_key.vtn_name))) {
      uuu::upll_strncpy(
      key2->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
      vrt_if_key->flowfilter_key.if_key.vrt_key.vtn_key.vtn_name,
      (kMaxLenVtnName + 1));
      rename |= VTN_RENAME;
    }
    if (strcmp(reinterpret_cast<char *>(key2->
               flowfilter_key.if_key.vrt_key.vrouter_name),
               reinterpret_cast<const char *>(vrt_if_key->
               flowfilter_key.if_key.vrt_key.vrouter_name))) {
      uuu::upll_strncpy(
      key2->flowfilter_key.if_key.vrt_key.vrouter_name,
      vrt_if_key->flowfilter_key.if_key.vrt_key.vrouter_name,
      (kMaxLenVnodeName + 1));
      rename |= VN_RENAME;
    }
   } else if (ikey->get_key_type() == UNC_KT_VTERMIF_FLOWFILTER_ENTRY) {
     UPLL_LOG_DEBUG("vterm_if_ff_entry1");
     key_vterm_if_flowfilter_entry_t *vterm_if_key = reinterpret_cast
      <key_vterm_if_flowfilter_entry_t *>(unc_key->get_key());
    if (strcmp(reinterpret_cast<char *>(key3->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name),
               reinterpret_cast<const char *>(vterm_if_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name))) {
      uuu::upll_strncpy(
      key3->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
      vterm_if_key->flowfilter_key.if_key.vterm_key.vtn_key.vtn_name,
      kMaxLenVtnName + 1);
      rename |= VTN_RENAME;
    }
    if (strcmp(reinterpret_cast<char *>(key3->flowfilter_key.if_key.vterm_key.vterminal_name),
               reinterpret_cast<const char *>(vterm_if_key->flowfilter_key.if_key.vterm_key.vterminal_name))) {
      uuu::upll_strncpy(
      key3->flowfilter_key.if_key.vterm_key.vterminal_name,
      vterm_if_key->flowfilter_key.if_key.vterm_key.vterminal_name,
      kMaxLenVnodeName + 1);
      rename |= VN_RENAME;
    }
   }
   SET_USER_DATA(ikey, unc_key);
   SET_USER_DATA_FLAGS(ikey, rename);
  }
  mgr = NULL;
  DELETE_IF_NOT_NULL(unc_key);

  val_flowfilter_entry_t *val_flowfilter_entry =
      reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
  if (!val_flowfilter_entry) {
    UPLL_LOG_ERROR("val_flowfilter_entry NULL");
    return UPLL_RC_SUCCESS;
  }

  if (UNC_VF_VALID == val_flowfilter_entry
                      ->valid[UPLL_IDX_FLOWLIST_NAME_FFE]) {
    val_rename_flowlist_t *rename_flowlist =
      reinterpret_cast<val_rename_flowlist_t*>
                   (ConfigKeyVal::Malloc(sizeof(val_rename_flowlist_t)));
    if (!rename_flowlist) {
      UPLL_LOG_DEBUG("rename_flowlist NULL %d", result_code);
      return UPLL_RC_ERR_GENERIC;
    }

    uuu::upll_strncpy(rename_flowlist->flowlist_newname,
                    val_flowfilter_entry->flowlist_name,
                    (kMaxLenFlowListName + 1));
  rename_flowlist->valid[UPLL_IDX_RENAME_FLOWLIST_RFL] = UNC_VF_VALID;

  MoMgrImpl* mgr = reinterpret_cast<MoMgrImpl *>
         (const_cast<MoManager *> (GetMoManager(UNC_KT_FLOWLIST)));
  if (!mgr) {
    UPLL_LOG_DEBUG("mgr failed");
    if (rename_flowlist) free(rename_flowlist);
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = mgr->GetChildConfigKey(unc_key, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("GetChildConfigKey Returned an error");
    free(rename_flowlist);
    mgr = NULL;
    return result_code;
  }
  SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
  unc_key->AppendCfgVal(IpctSt::kIpcStValRenameFlowlist, rename_flowlist);
  result_code = mgr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
      RENAMETBL);
  if ((UPLL_RC_SUCCESS != result_code) &&
      (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
      UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
      DELETE_IF_NOT_NULL(unc_key);
      mgr = NULL;
      return result_code;
    }

    if (result_code == UPLL_RC_SUCCESS) {
      key_flowlist_t *key_flowlist = NULL;
      key_flowlist = reinterpret_cast<key_flowlist_t *> (unc_key->get_key());

      uuu::upll_strncpy(val_flowfilter_entry->flowlist_name,
                      key_flowlist->flowlist_name,
                      (kMaxLenFlowListName + 1));
      rename |= FF_RENAME;
      SET_USER_DATA(ikey, unc_key);
      SET_USER_DATA_FLAGS(ikey, rename);
    }
    DELETE_IF_NOT_NULL(unc_key);
    mgr = NULL;
  }
  UPLL_LOG_TRACE("%s GetRenamedUncKey vbrff_entry end",
                  ikey->ToStrAll().c_str());
  DELETE_IF_NOT_NULL(unc_key);
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::GetRenamedUncKeyWithRedirection(unc_key_type_t kt_type,
                                                     const char *ctrlr_id,
                                                     DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_ERR_GENERIC;

  upll_keytype_datatype_t dt_type = UPLL_DT_IMPORT;
  ConfigKeyVal *unc_key = NULL, *ikey = NULL;
  if (kt_type != UNC_KT_VBR_FLOWFILTER_ENTRY &&
      kt_type != UNC_KT_VBRIF_FLOWFILTER_ENTRY &&
      kt_type != UNC_KT_VRTIF_FLOWFILTER_ENTRY &&
      kt_type != UNC_KT_VTERMIF_FLOWFILTER_ENTRY) {
    UPLL_LOG_ERROR("Invalid keytype for Redirection %d", kt_type);
    return UPLL_RC_ERR_GENERIC;
  }
  if ((ctrlr_id == NULL) || (NULL == dmi)) {
    UPLL_LOG_DEBUG("ikey/ctrlr_id dmi NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  result_code = GetChildConfigKey(ikey, NULL);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_INFO("GetChildConfigKey failed %d", result_code);
    return result_code;
  }
  SET_USER_DATA_CTRLR(ikey, ctrlr_id);
  DbSubOp dbop = {kOpReadMultiple, kOpMatchCtrlr, 
                  kOpInOutCtrlr|kOpInOutDomain|kOpInOutFlag };
  result_code = ReadConfigDB(ikey, dt_type, UNC_OP_READ, dbop, dmi,
                             MAINTBL);
  if (UPLL_RC_SUCCESS != result_code &&
      UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
    UPLL_LOG_ERROR("ReadConfigDB failed %d", result_code);
    delete ikey;
    return result_code;
  }
  if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
      delete ikey;
      return UPLL_RC_SUCCESS;
  }
  ConfigKeyVal *temp_ckv = ikey;
  while (ikey) {
    val_flowfilter_entry_t *val_flowfilter_entry =
      reinterpret_cast<val_flowfilter_entry_t *> (GetVal(ikey));
    if (!val_flowfilter_entry) {
      UPLL_LOG_ERROR("val_flowfilter_entry NULL");
      delete temp_ckv;
      return UPLL_RC_SUCCESS;
    }

    if (((UNC_VF_VALID ==
            val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE]) ||
          (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_NODE_FFE] ==
           UNC_VF_VALUE_NOT_MODIFIED)) &&
        ((UNC_VF_VALID ==
          val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE])||
         (val_flowfilter_entry->valid[UPLL_IDX_REDIRECT_PORT_FFE] ==
          UNC_VF_VALUE_NOT_MODIFIED))) {
      MoMgrImpl *mgrvbr = reinterpret_cast<MoMgrImpl *>(
          const_cast<MoManager *>(GetMoManager(UNC_KT_VBRIDGE)));
      if (!mgrvbr) {
        UPLL_LOG_TRACE("mgrvbr failed");
        delete temp_ckv;
        return UPLL_RC_ERR_GENERIC;
      }
      val_rename_vnode *rename_val = reinterpret_cast<val_rename_vnode*>
        (ConfigKeyVal::Malloc(sizeof(val_rename_vnode)));
      
      uuu::upll_strncpy(rename_val->ctrlr_vnode_name,
          val_flowfilter_entry->redirect_node,
          (kMaxLenVnodeName + 1));
      rename_val->valid[UPLL_CTRLR_VNODE_NAME_VALID] = UNC_VF_VALID;

      result_code = mgrvbr->GetChildConfigKey(unc_key, NULL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("GetChildConfigKey Returned an error");
        FREE_IF_NOT_NULL(rename_val);
        mgrvbr = NULL;
        delete temp_ckv;
        return result_code;
      }
      key_vbr_t *vbr_key = reinterpret_cast<key_vbr_t *>(unc_key->get_key());

      if (kt_type == UNC_KT_VBR_FLOWFILTER_ENTRY) {
        uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
            reinterpret_cast<key_vbr_flowfilter_entry_t *>(
              ikey->get_key())->flowfilter_key.vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName+1)); 
      } else {
        uuu::upll_strncpy(vbr_key->vtn_key.vtn_name,
            reinterpret_cast<key_vbr_if_flowfilter_entry_t *>(ikey->get_key())->
            flowfilter_key.if_key.vbr_key.vtn_key.vtn_name,
            (kMaxLenVtnName+1)); 
      }
      SET_USER_DATA_CTRLR(unc_key, ctrlr_id);
      unc_key->AppendCfgVal(IpctSt::kIpcStValRenameVtn, rename_val);
      DbSubOp dbop = { kOpReadSingle, kOpMatchCtrlr, kOpInOutNone };
      /*
       * UNC VTN NAME, PFC_VNODE_NAME and CTRLR_NAME only enough for check exists in 
       * rename tabel or not
       */
      result_code = mgrvbr->ReadConfigDB(unc_key, dt_type, UNC_OP_READ, dbop, dmi,
          RENAMETBL);
      if ((UPLL_RC_SUCCESS != result_code) &&
          (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
        UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
        DELETE_IF_NOT_NULL(unc_key);
        mgrvbr = NULL;
        delete temp_ckv;
        return result_code;
      }
      if (result_code == UPLL_RC_SUCCESS) {
          key_vbr *vbr_key = reinterpret_cast<key_vbr *>(unc_key->get_key());
          uuu::upll_strncpy(val_flowfilter_entry->redirect_node,
              vbr_key->vbridge_name,
              (kMaxLenVnodeName + 1));
          result_code = UpdateConfigDB(ikey, UPLL_DT_IMPORT, UNC_OP_UPDATE, dmi);
          if (UPLL_RC_SUCCESS != result_code) {
            UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
            DELETE_IF_NOT_NULL(unc_key);
            mgrvbr = NULL;
            delete temp_ckv;
            return result_code;
          }
      }
      DELETE_IF_NOT_NULL(unc_key);
      mgrvbr = NULL;
    }
    ikey = ikey->get_next_cfg_key_val();
  }
  DELETE_IF_NOT_NULL(temp_ckv);
  return UPLL_RC_SUCCESS;
}

}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc



