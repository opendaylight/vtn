/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <time.h>
//  #include <iostream>
#include <string>
#include <list>
#include <map>
#include "pfc/log.h"
#include "unc/uppl_common.h"
#include "momgr_impl.hh"
#include "dhcprelay_server_momgr.hh"
// #include "vtn_momgr.hh"
// #include "vbr_momgr.hh"
#include "vlink_momgr.hh"
// #include "vbr_if_momgr.hh"
//  #include "policingprofile_entry_momgr.hh"
#include "uncxx/upll_log.hh"

// using namespace std;
using unc::upll::ipc_util::IpcUtil;
using unc::upll::dal::DalResultCode;
// map<unc_key_type_t,  MoMgr *> MoMgr::momgr;

// map<unc_key_type_t,  MoMgr *> MoMgr::momgr;

namespace unc {
namespace upll {
namespace kt_momgr {


#define KEY_TYPE_BIND_CS(key_type) ((key_type == UNC_KT_FLOWLIST) || \
                                    (key_type == UNC_KT_FLOWLIST_ENTRY) || \
                                    (key_type == UNC_KT_VRTIF_FLOWFILTER) || \
                                    (key_type == UNC_KT_VBR_FLOWFILTER) || \
                                    (key_type == UNC_KT_VBRIF_FLOWFILTER) || \
                                    (key_type == UNC_KT_POLICING_PROFILE) || \
                                    (key_type == UNC_KT_VTN_POLICINGMAP) || \
                                    (key_type == UNC_KT_VBR_POLICINGMAP) || \
                                    (key_type == UNC_KT_VBRIF_POLICINGMAP) || \
                                    (key_type == UNC_KT_VTN_FLOWFILTER))

upll_rc_t MoMgrImpl::CreateMo(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi) {
//  std::cout << "MoMgrImpl::createMo ";
  if (ikey == NULL || req == NULL) {
//    std::cout << "Improper inputs : Cannot create object";
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
                                    const char *domain_id) {
  UPLL_FUNC_TRACE;
//  std::cout << "MoMgrImpl::createMo ";
  if (ikey == NULL || req == NULL) {
//    std::cout << "Improper inputs : Cannot create object";
    return UPLL_RC_ERR_GENERIC;
  }
  switch (req->datatype) {
    case UPLL_DT_AUDIT: {
//      std::cout << "UPLL_DT_AUDIT";
      return CreateAuditMoImpl(ikey, dmi, ctrlr_id);
    }
    case UPLL_DT_IMPORT: {
      return CreateMo(req, ikey, dmi);
    }
    default:
      return UPLL_RC_ERR_GENERIC;
  }
}

upll_rc_t MoMgrImpl::CreateCandidateMo(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
//  std::cout << "MoMgrImpl::CreateCandidateMo";
  if (ikey == NULL || req == NULL) {
        return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // validate syntax and semantics
  result_code = ValidateMessage(req, ikey);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateMessage failed, Error - %d", result_code);
    return result_code;
  }
  result_code = ValidateAttribute(ikey, dmi, req);
  if (result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("ValidateAttribute failed, Error - %d", result_code);
    return result_code;
  }

  // Parent check
  if (ikey->get_key_type() == UNC_KT_VUNKNOWN ||
    ikey->get_key_type() == UNC_KT_VUNK_IF) {
    ConfigKeyVal *okey = NULL;
    if (ikey->get_key_type() == UNC_KT_VUNKNOWN) {
      // Vnode Existence check in CANDIDATE DB
      result_code = VnodeChecks(ikey, req->datatype, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(
            "Another Vnode with the same name already exists. Error code : %d",
             result_code);
        return result_code;
      }
      result_code = GetParentConfigKey(okey, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in retrieving the parent ConfigKeyVal");
        if (okey)
          delete okey;
        return UPLL_RC_ERR_GENERIC;
      }
      MoMgrImpl *vtn_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                   (const_cast<MoManager*>(GetMoManager(UNC_KT_VTN))));
      result_code = vtn_mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                    dmi, MAINTBL);
      if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
        UPLL_LOG_DEBUG("Parent doesn't exist!");
        if (okey)
          delete okey;
        if (UPLL_RC_ERR_NO_SUCH_INSTANCE == result_code) {
          return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
        }else { 
          UPLL_LOG_DEBUG("UpdateConfigDB Failed %d", result_code);
          return result_code;
        }
      }
      if (okey)
      delete okey;
    }
    if (ikey->get_key_type() == UNC_KT_VUNK_IF) {
      result_code = GetParentConfigKey(okey, ikey);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Error in retrieving the parent ConfigKeyVal");
        if (okey)
          delete okey;
        return UPLL_RC_ERR_GENERIC;
      }
      MoMgrImpl *vunk_mgr = reinterpret_cast<MoMgrImpl *>(const_cast<MoManager *>
                   (const_cast<MoManager*>(GetMoManager(UNC_KT_VUNKNOWN))));
      result_code = vunk_mgr->UpdateConfigDB(okey, req->datatype, UNC_OP_READ,
                                    dmi, MAINTBL);
      if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
        UPLL_LOG_DEBUG("Parent doesn't exist!");
        if (okey)
          delete okey;
        return UPLL_RC_ERR_PARENT_DOES_NOT_EXIST;
      }
      if (okey)
      delete okey;
    }
  }
  // Check if VTN already exists in CANDIDATE DB
  result_code = UpdateConfigDB(ikey, req->datatype, UNC_OP_READ, dmi);
  if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS
    || result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    return result_code;
  }
  if (UPLL_DT_CANDIDATE == req->datatype) {
    // Check if VTN exists in RUNNING DB and move it to CANDIDATE DB
    result_code = UpdateConfigDB(ikey, UPLL_DT_RUNNING,
                               UNC_OP_READ, dmi, MAINTBL);
    if (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code &&
      UPLL_RC_ERR_INSTANCE_EXISTS != result_code) {
      UPLL_LOG_DEBUG("Error in UpdateConfigDB %d", result_code);
        return result_code;
    }
    if (result_code == UPLL_RC_ERR_INSTANCE_EXISTS) {
      result_code = RestoreChildren(ikey, req->datatype, UPLL_DT_RUNNING, dmi);
      if (result_code != UPLL_RC_SUCCESS) {
        return result_code;
      }
      return result_code;
    }
  }
  // create a record in CANDIDATE DB
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
         UPLL_LOG_DEBUG("ReadInfoFromDB failed, result_code= %d",
                    result_code);
         return result_code;
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
      if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        UPLL_LOG_DEBUG("Failed to ReadConfigDb- %d", result_code);
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
      return child_result_code;
    }
  }

  UPLL_LOG_TRACE("Before AdaptValtovtnservice  %s", (ikey->ToStrAll()).c_str());
  if (result_code == UPLL_RC_SUCCESS && UPLL_DT_IMPORT != header->datatype) {
    result_code = AdaptValToVtnService(ikey);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d",
                    result_code);
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
          return UPLL_RC_ERR_GENERIC;
        }
        result_code = mgr_dhcp->GetVrtDhcpRelayServerAddress(one_ckv, dmi);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetVrtDhcpRelayServerAddress Failed");
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
        return result_code;
      }
      result_code = ReadConfigDB(temp_key, header->datatype,
                                              UNC_OP_READ, dbop1, dmi, MAINTBL);
      if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("ReadConfigDB failed %d", result_code);
        DELETE_IF_NOT_NULL(temp_key);
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
        result_code = AdaptValToVtnService(start_key);

      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d",
                    result_code);
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
  upll_rc_t result_code = DeleteChildren(ikey, req->datatype, dmi, MAINTBL);
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
  UPLL_LOG_INFO("UpdateMo for %d", ikey->get_key_type());
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
  if (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("ReadConfigDB Failed %d", result_code);
    delete temp_ck;
    return result_code;
  }
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

    if (driver_result == UPLL_RC_ERR_RESOURCE_DISCONNECTED) {
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
      /* if renamed obtain the unc specifc name */
      result_code = GetRenamedUncKey(err_key, UPLL_DT_CANDIDATE, dmi, ctrlr_id);
      if ((result_code != UPLL_RC_SUCCESS) &&
         (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)) {
        UPLL_LOG_TRACE("Error in getting renamed name ");
        continue;
      }
      /* Verifying for Boundary VLink */
      if (err_key->get_key_type() == UNC_KT_VBR_IF) {
        ConfigKeyVal *ck_vlink = NULL;
        VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>(
              const_cast<MoManager *>(GetMoManager(UNC_KT_VLINK)));
        if (!mgr) {
          UPLL_LOG_DEBUG("Invalid mgr");
          return UPLL_RC_ERR_GENERIC;
        }
        vn_if_type iftype;
        result_code = mgr->CheckIfMemberOfVlink(err_key,
                                     UPLL_DT_CANDIDATE, ck_vlink, dmi, iftype);
        /* replace err_key with ck_vlink */
        if (result_code == UPLL_RC_SUCCESS) {
          if ((iftype == kVlinkBoundaryNode1) || (iftype == kVlinkBoundaryNode2)) {
            err_key->ResetWith(ck_vlink);
            continue;
          } else {
            if (ck_vlink) delete ck_vlink;
            UPLL_LOG_TRACE("%s is an internal vlink", (err_key->ToStr()).c_str());
          }
        } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_DEBUG("Returning error %d", result_code);
            if (ck_vlink) delete ck_vlink;
            return UPLL_RC_ERR_GENERIC;
        } else {
          if (ck_vlink) delete ck_vlink;
          UPLL_LOG_TRACE("%s is not part of a vlink", (err_key->ToStr()).c_str());
        }
      }
      result_code = AdaptValToVtnService(err_key);
    }
  }
  UPLL_LOG_TRACE("TxVote Controller Status : %d", driver_result);
  if (!keytype_no_err)
    return UPLL_RC_SUCCESS;
  // Return vote Result to UPLLManager
  driver_result = (driver_result == UPLL_RC_ERR_RESOURCE_DISCONNECTED)?
                UPLL_RC_SUCCESS : driver_result;
  // return driver_result;
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ?
      UPLL_RC_SUCCESS : result_code;
  return result_code;
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
  DalResultCode db_result;
  ConfigKeyVal *req = NULL, *nreq = NULL, *ck_main = NULL;
  controller_domain_t ctrlr_dom, vlink_ctrlr_dom[2];
  vlink_ctrlr_dom[0].ctrlr = NULL;
  vlink_ctrlr_dom[0].domain = NULL;
  vlink_ctrlr_dom[1].ctrlr = NULL;
  vlink_ctrlr_dom[1].domain = NULL;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  DalCursor *dal_cursor_handle = NULL;
  IpcResponse ipc_resp;
  if (affected_ctrlr_set == NULL)
      return UPLL_RC_ERR_GENERIC;
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
               ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
               ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));

  if ((UNC_OP_UPDATE == op) && ((UNC_KT_VBR_FLOWFILTER == keytype) ||
      (UNC_KT_VBRIF_FLOWFILTER == keytype) ||
      (UNC_KT_VRTIF_FLOWFILTER == keytype))) {
      // Update operation not supported.
      // return success
    return UPLL_RC_SUCCESS;
  }
  result_code = DiffConfigDB(UPLL_DT_CANDIDATE, UPLL_DT_RUNNING,
                     op, req, nreq, &dal_cursor_handle, dmi, MAINTBL);
  while (result_code == UPLL_RC_SUCCESS) {
      //  Get Next Record
    db_result = dmi->GetNextRecord(dal_cursor_handle);
    result_code = DalToUpllResCode(db_result);
    if (result_code != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG(" GetNextRecord failed err code(%d)", result_code);
      break;
    }
    ck_main = NULL;
    switch (op)   {
      case UNC_OP_CREATE:
      case UNC_OP_UPDATE:
     /* fall through intended */
        result_code = DupConfigKeyVal(ck_main, req);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_TRACE("DupConfigKeyVal failed %d", result_code);
          dmi->CloseCursor(dal_cursor_handle, true);
          delete req;
          if (nreq) delete nreq;
          return result_code;
        }
        break;
     case UNC_OP_DELETE:
       result_code = GetChildConfigKey(ck_main, req);
       if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_TRACE("GetChildConfigKey failed %d", result_code);
         dmi->CloseCursor(dal_cursor_handle, true);
         delete req;
         if (nreq) delete nreq;
         return result_code;
       }
       break;
     default:
       return UPLL_RC_ERR_GENERIC;
    }
    /* Boundary VLink is not sent to Controller*/
    if (ck_main->get_key_type() == UNC_KT_VLINK && (op != UNC_OP_DELETE)) {
      bool bound_vlink = false;
      VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>(
            const_cast<MoManager *>(GetMoManager(UNC_KT_VLINK)));
      if (!mgr) {
        UPLL_LOG_DEBUG("Invalid mgr");
        dmi->CloseCursor(dal_cursor_handle, true);
        delete req;
        if (nreq) delete nreq;
        delete ck_main;
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = mgr->BoundaryVlink(ck_main, vlink_ctrlr_dom, bound_vlink);
      if (bound_vlink) {
        UPLL_LOG_TRACE("%s is a boundary Link. Not Sent to Controller",
            (reinterpret_cast<key_vlink *>(ck_main->get_key()))->vlink_name);
        if (ck_main) delete ck_main;
        continue;
      }
    }
    GET_USER_DATA_CTRLR_DOMAIN(ck_main, ctrlr_dom);
    if ((op == UNC_OP_CREATE) || (op == UNC_OP_UPDATE)) {
      void *main = GetVal(ck_main);
      void *val_nrec = (nreq) ? GetVal(nreq) : NULL;
      if (FilterAttributes(main, val_nrec, false, op)) {
        if (ck_main) delete ck_main;
        continue;
      }
    }
    if (!OVERLAY_KT(keytype)) {
      upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
        UPLL_DT_RUNNING : UPLL_DT_CANDIDATE;
      if (ck_main->get_key_type() == UNC_KT_VLINK) {
        if (ck_main->get_cfg_val()) {
          GET_USER_DATA_CTRLR_DOMAIN(ck_main->get_cfg_val(), vlink_ctrlr_dom[1]);
        } else {
          vlink_ctrlr_dom[1].ctrlr = NULL;
          vlink_ctrlr_dom[1].domain = NULL;
        }
      }
      controller_domain_t *tctrlr_dom =
       (ck_main->get_key_type() == UNC_KT_VLINK)?
                     &vlink_ctrlr_dom[1]:&ctrlr_dom;
      result_code = GetRenamedControllerKey(ck_main, dt_type,
                                            dmi, tctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG(" GetRenamedControllerKey failed err code(%d)",
                        result_code);
         if (ck_main)
           delete ck_main;
         break;
      }
      if (ck_main->get_key_type() == UNC_KT_VLINK && (op == UNC_OP_DELETE)) {
        /* boundary vlink */
        VlinkMoMgr *vlink_mgr = reinterpret_cast<VlinkMoMgr *>(
              const_cast<MoManager *>(GetMoManager(UNC_KT_VLINK)));
        if (!vlink_mgr) {
          UPLL_LOG_DEBUG("Invalid mgr");
          if (ck_main) delete ck_main;
          DELETE_IF_NOT_NULL(req);
          dmi->CloseCursor(dal_cursor_handle, true);
          return UPLL_RC_ERR_GENERIC;
        }
        unc_key_type_t ktype[2] = {UNC_KT_ROOT, UNC_KT_ROOT};
        for (int vnode_count = 0; vnode_count < 2; vnode_count++)
          ktype[vnode_count] = vlink_mgr->
                               GetVlinkVnodeIfKeyType(ck_main, vnode_count);
        if (ktype[0] == UNC_KT_VUNK_IF || ktype[1] == UNC_KT_VUNK_IF ||
           strncmp(reinterpret_cast<const char *>(ctrlr_dom.ctrlr), 
                   reinterpret_cast<const char *>(tctrlr_dom->ctrlr), 
                   kMaxLenCtrlrId + 1) ||
           strncmp(reinterpret_cast<const char *>(ctrlr_dom.domain), 
                   reinterpret_cast<const char *>(tctrlr_dom->domain), 
                   kMaxLenDomainId + 1)) {
          UPLL_LOG_TRACE("%s is a boundary Link. Not Sent to Controller/n",
            (reinterpret_cast<key_vlink *>(ck_main->get_key()))->vlink_name);
          if (ck_main)
            delete ck_main;
          continue;
        }
      }
    } else if  (op == UNC_OP_DELETE) {
      UPLL_LOG_TRACE("Overlay KeyType %d", keytype);
      DbSubOp dbop = {kOpReadSingle, kOpMatchNone,
                      kOpInOutCtrlr | kOpInOutDomain};
      result_code = ReadConfigDB(ck_main, UPLL_DT_RUNNING,
                                 UNC_OP_READ, dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ReadConfigDB from running failed %d", result_code);
        DELETE_IF_NOT_NULL(req);
        dmi->CloseCursor(dal_cursor_handle, true);
        return UPLL_RC_ERR_GENERIC;
      }
      if (keytype == UNC_KT_VTUNNEL || keytype == UNC_KT_VTUNNEL_IF) {
        result_code = PopulateDriverDeleteCkv(ck_main, dmi, UPLL_DT_RUNNING);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Couldnt form Driver Val structure %d", result_code);
          if (ck_main) delete ck_main;
          DELETE_IF_NOT_NULL(req);
          dmi->CloseCursor(dal_cursor_handle, true);
          return UPLL_RC_ERR_GENERIC;
        }
      }
    }
    UPLL_LOG_TRACE("Controller : %s; Domain : %s", ctrlr_dom.ctrlr,
                 ctrlr_dom.domain);
    // Inserting the controller to Set
    affected_ctrlr_set->insert
      (string(reinterpret_cast<char *>(ctrlr_dom.ctrlr)));
    result_code = SendIpcReq(session_id, config_id, op, UPLL_DT_CANDIDATE,
      ck_main, &ctrlr_dom, &ipc_resp);

    if (result_code == UPLL_RC_ERR_RESOURCE_DISCONNECTED) {
      UPLL_LOG_DEBUG(" driver result code - %d", ipc_resp.header.result_code);
      result_code = UPLL_RC_SUCCESS;
    }
    if (result_code != UPLL_RC_SUCCESS) {
      // If Driver returned error, return the err_ckv received from driver after
      // getting its UNC key and apdapting the CKV to North Bound API
      // If update to driver failed, exit the loop.
      UPLL_LOG_DEBUG("IpcSend failed %d", result_code);
      upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
        UPLL_DT_RUNNING : UPLL_DT_CANDIDATE;
      upll_rc_t local_rc = GetRenamedUncKey(ipc_resp.ckv_data, dt_type, dmi,
                                     ctrlr_dom.ctrlr);
      if (UPLL_RC_SUCCESS != local_rc &&
          UPLL_RC_ERR_NO_SUCH_INSTANCE != local_rc) {
        UPLL_LOG_DEBUG("GetRenamedUncKey failed %d", local_rc);
        DELETE_IF_NOT_NULL(ck_main);
        DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
	result_code = UPLL_RC_ERR_GENERIC;
        break;
      }
      if (ipc_resp.ckv_data) {
        upll_rc_t local_rc = AdaptValToVtnService(ipc_resp.ckv_data);
        if (local_rc != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("AdaptValToVtnService failed result_code %d",
                          local_rc);
          DELETE_IF_NOT_NULL(ck_main);
          DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
	  result_code = UPLL_RC_ERR_GENERIC;
          break;
        }
        *err_ckv = ipc_resp.ckv_data;
      }
      if (ck_main)
        delete ck_main;
      break;
    }
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    DELETE_IF_NOT_NULL(ck_main);
  }
  if (dal_cursor_handle)
    dmi->CloseCursor(dal_cursor_handle, true);
  if (req)
    delete req;
  if (nreq)
    delete nreq;
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
  if (rc != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Request for Key %d failed  with error %d",
                  ckv->get_key_type(), rc);
  }
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

  if ((ctrlr_commit_status == NULL) || (dmi == NULL))
    return UPLL_RC_ERR_GENERIC;
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
          result_code = GetRenamedUncKey(ck_err, UPLL_DT_CANDIDATE,
                                       dmi, ctrlr_id);
          if (result_code != UPLL_RC_SUCCESS &&
              result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_DEBUG("GetRenamedUncKey failed. Result: %d", result_code);
            return result_code;
          }
        }
        if (ck_err->get_key_type() == UNC_KT_VBR_IF) {
          ConfigKeyVal *ck_vlink = NULL;
          VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>(
              const_cast<MoManager *>(GetMoManager(UNC_KT_VLINK)));
          if (!mgr) {
            UPLL_LOG_DEBUG("Invalid mgr");
            return UPLL_RC_ERR_GENERIC;
          }
          vn_if_type iftype;
          result_code = mgr->CheckIfMemberOfVlink(ck_err,
                                       UPLL_DT_CANDIDATE, ck_vlink, dmi, iftype);
         /* replace ck_err with ck_vlink */
          if (result_code == UPLL_RC_SUCCESS) {
            if ((iftype == kVlinkBoundaryNode1) ||
                (iftype == kVlinkBoundaryNode2)) {
              ck_err->ResetWith(ck_vlink);
              continue;
            } else {
              if (ck_vlink) delete ck_vlink;
              UPLL_LOG_DEBUG("%s is an internal vlink", (ck_err->ToStr()).c_str());
            }
          } else if (result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) {
            UPLL_LOG_DEBUG("Returning error %d", result_code);
            if (ck_vlink) delete ck_vlink;
            result_code = AdaptValToVtnService(ck_err);
            return UPLL_RC_ERR_GENERIC;
          } else {
            if (ck_vlink) delete ck_vlink;
            UPLL_LOG_TRACE("%s is not part of a vlink", (ck_err->ToStr()).c_str());
          }
        }
        result_code = AdaptValToVtnService(ck_err);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Returning error %d\n", result_code);
          return result_code;
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
        driver_result = ctrlr_result[controller];
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
             uint32_t cons_result = ctrlr_result[controller2] |
                                    ctrlr_result[controller];
             UpdateConfigStatus(ckey, op[i], cons_result, nreq, dmi);
           }  else {
             UpdateConfigStatus(ckey, op[i], ctrlr_result[controller],
                                nreq, dmi);
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
           result_code = UpdateParentOperStatus(ckey,dmi);
           if (result_code != UPLL_RC_SUCCESS) {
             delete ckey;
             if (cfg1_cursor)
               dmi->CloseCursor(cfg1_cursor, true);
             UPLL_LOG_DEBUG("Returning error %d\n",result_code);
             return result_code;
           }
         }
      }
      DbSubOp dbop_update = {kOpNotRead, kOpMatchNone, kOpInOutCs |
                             kOpInOutFlag | kOpInOutCtrlr | kOpInOutDomain};
      result_code = UpdateConfigDB(ckey, dt_type, op[i], dmi, &dbop_update,
                                                                  MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        delete ckey;
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
  if (!no_rename) {
    switch (dup_ikey->get_key_type()) {
    case UNC_KT_VBRIDGE:
    case UNC_KT_VROUTER:
    case UNC_KT_VLINK:
        result_code =  VnodeChecks(okey, req->datatype, dmi);
        if (UPLL_RC_SUCCESS != result_code) {
           DELETE_IF_NOT_NULL(dup_ikey);
           DELETE_IF_NOT_NULL(okey);
           return UPLL_RC_ERR_GENERIC;
        }
        result_code =  VnodeChecks(okey, UPLL_DT_CANDIDATE, dmi);
        if (UPLL_RC_SUCCESS != result_code) {
           DELETE_IF_NOT_NULL(dup_ikey);
           DELETE_IF_NOT_NULL(okey);
           return UPLL_RC_ERR_GENERIC;
        }
    default:
       break;
    }
  }
  // Set The controller ID into the dup_ikey & okey
  SET_USER_DATA_CTRLR(dup_ikey, ctrlr_id);
  SET_USER_DATA_CTRLR(okey, ctrlr_id);
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
      /* Ensure the renamed name does not correspond to an existing UNC name */
    if (!no_rename) {
      result_code = UpdateConfigDB(okey, UPLL_DT_CANDIDATE, UNC_OP_READ,
                                   dmi, MAINTBL);
      if (UPLL_RC_ERR_INSTANCE_EXISTS != result_code &&
          UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
         UPLL_LOG_DEBUG("UpdateConfigDB Failed %d", result_code);
         DELETE_IF_NOT_NULL(okey);
         DELETE_IF_NOT_NULL(dup_ikey);
         return result_code;
      }
      if (UPLL_RC_ERR_INSTANCE_EXISTS == result_code &&
         UNC_KT_VTN != okey->get_key_type()) {
         UPLL_LOG_DEBUG("Record Exists in DT CANDIDATE %d ", result_code);
         DELETE_IF_NOT_NULL(okey);
         DELETE_IF_NOT_NULL(dup_ikey);
         return result_code;
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
                                            DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    DalResultCode dal_result = uud::kDalRcSuccess;
    UPLL_LOG_TRACE("MergeImportToCandidate ");
    for (int tbl = MAINTBL; tbl < ntable; tbl++) {
      uudst::kDalTableIndex tbl_index;
      tbl_index = GetTable((MoMgrTables)tbl, UPLL_DT_IMPORT);
      /* skipping VTN Main Table */
      if (tbl_index < uudst::kDalNumTables && tbl_index != 0) {
        UPLL_LOG_TRACE("Merging the %d Table ", tbl);
        if (table[tbl]->get_key_type() != keytype)
          return UPLL_RC_ERR_GENERIC;
        DalBindInfo *dal_bind_info = new DalBindInfo(tbl_index);
        dal_result = dmi->CopyModifiedInsertRecords(UPLL_DT_CANDIDATE,
                      UPLL_DT_IMPORT, tbl_index, dal_bind_info);
        UPLL_LOG_TRACE("%d Table is completed ", tbl);
        if (dal_result != uud::kDalRcSuccess) {
          delete dal_bind_info;
          break;
        }
        delete dal_bind_info;
      }
    }
    // convert dal_result to result_code
    return DalToUpllResCode(dal_result);
}

upll_rc_t MoMgrImpl::ImportClear(unc_key_type_t keytype,
                                 const char *ctrlr_id,
                                 DalDmlIntf *dmi) {
  // UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  // cout<<" ImportClear ";
  for (int tbl = MAINTBL; tbl < ntable; tbl++) {
    uudst::kDalTableIndex tbl_index;
    tbl_index = GetTable((MoMgrTables)tbl, UPLL_DT_IMPORT);
      if ((tbl_index < uudst::kDalNumTables)) {
        result_code = DalToUpllResCode(dmi->DeleteRecords(UPLL_DT_IMPORT,
                                          tbl_index, NULL));
        if ((UPLL_RC_SUCCESS != result_code) &&
          (UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code)) {
          UPLL_LOG_DEBUG("DeleteRecords Failed %d", result_code);
          return result_code;
        }
      }
    }
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::GetDiffRecord(ConfigKeyVal *ckv_running,
                                   ConfigKeyVal *ckv_audit,
                                   uuc::UpdateCtrlrPhase phase, MoMgrTables tbl,
                                   ConfigKeyVal *&okey,
                                   DalDmlIntf *dmi,
                                   bool &invalid_attr) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  ConfigKeyVal *ckv_dup = NULL;
  okey = NULL;
  DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutCtrlr | kOpInOutDomain|
                                               kOpInOutFlag | kOpInOutCs};
  switch (phase) {
    case uuc::kUpllUcpDelete:
      UPLL_LOG_TRACE("Deleted record is %s ", ckv_running->ToStrAll().c_str());
      result_code = GetChildConfigKey(okey, ckv_running);
      if (result_code != UPLL_RC_SUCCESS) {
         UPLL_LOG_DEBUG("GetChildConfigKey failed. err_code & phase %d %d",
                           result_code, phase);
         return result_code;
      }
    break;
    case uuc::kUpllUcpCreate:
      if (tbl == CTRLRTBL) {
        UPLL_LOG_TRACE("Created  record fot ctrlr_tbl is %s ", ckv_running->ToStrAll().c_str());
        dbop.inoutop = kOpInOutCtrlr | kOpInOutDomain;
        result_code = GetChildConfigKey(okey, ckv_running);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetChildConfigKey failed. err_code & phase %d %d",
                           result_code, phase);
          return result_code;
        }
        result_code = ReadConfigDB(okey, UPLL_DT_RUNNING,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadConfigDB failed. err_code & phase %d %d",
                           result_code, phase);
          return result_code;
        }
      } else {
          UPLL_LOG_TRACE("Created  record is %s ", ckv_running->ToStrAll().c_str());
          result_code = DupConfigKeyVal(okey, ckv_running, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed. err_code & phase %d %d",
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
          UPLL_LOG_DEBUG("GetChildConfigKey for running record failed. "
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
          UPLL_LOG_DEBUG("GetChildConfigKey for audit record failed. "
                         "err_code & phase %d %d", result_code, phase);
          if (ckv_dup)
            delete ckv_dup;
          return result_code;
        }
        result_code = ReadConfigDB(ckv_dup, UPLL_DT_AUDIT,
                                     UNC_OP_READ, dbop, dmi, MAINTBL);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("ReadConfigDB from audit failed. "
                         "err_code & phase %d %d", result_code, phase);
          delete ckv_dup;
          return result_code;
        }
      } else {
          UPLL_LOG_TRACE("UpdateRecord  record  is %s ", ckv_running->ToStrAll().c_str());
          UPLL_LOG_TRACE("UpdateRecord  record  is %s ", ckv_audit->ToStrAll().c_str());
          result_code = DupConfigKeyVal(okey, ckv_running, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed for running record. "
                           "err_code & phase %d %d", result_code, phase);
            delete ckv_dup;
            return result_code;
          }
          result_code = DupConfigKeyVal(ckv_dup, ckv_audit, tbl);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_DEBUG("DupConfigKeyVal failed for audit record. "
                           "err_code & phase %d %d", result_code, phase);
            if (ckv_dup)
              delete ckv_dup;
            return result_code;
          }
      }
      if (GetVal(okey) != NULL &&
          GetVal(ckv_dup) != NULL) {
        void *val1 = GetVal(okey);
        invalid_attr = FilterAttributes(val1, GetVal(ckv_dup), false,
                         UNC_OP_UPDATE);
      }
    break;
    default:
      UPLL_LOG_DEBUG("Invalid operation %d", phase);
      return UPLL_RC_ERR_NO_SUCH_OPERATION;
      break;
  }
    delete ckv_dup;
  return result_code;
}

upll_rc_t MoMgrImpl::AuditUpdateController(unc_key_type_t keytype,
                             const char *ctrlr_id,
                             uint32_t session_id,
                             uint32_t config_id,
                             uuc::UpdateCtrlrPhase phase,
                             bool *ctrlr_affected,
                             DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result = uud::kDalRcSuccess;
  MoMgrTables tbl  = MAINTBL;
  controller_domain_t ctrlr_dom, vlink_ctrlr_dom[2];
  vlink_ctrlr_dom[0].ctrlr = NULL;
  vlink_ctrlr_dom[0].domain = NULL;
  vlink_ctrlr_dom[1].ctrlr = NULL;
  vlink_ctrlr_dom[1].domain = NULL;
  ctrlr_dom.ctrlr = NULL;
  ctrlr_dom.domain = NULL;
  ConfigKeyVal  *ckv_running = NULL;
  ConfigKeyVal  *ckv_audit = NULL;
  ConfigKeyVal  *ckv_drvr = NULL;
  ConfigKeyVal  *resp = NULL;
  DalCursor *cursor = NULL;
  bool invalid_attr = false;
  uint8_t *in_ctrlr = reinterpret_cast<uint8_t *>(const_cast<char *>(ctrlr_id));
  /* decides whether to retrieve from controller table or main table */
  GET_TABLE_TYPE(keytype, tbl);
  unc_keytype_operation_t op = (phase == uuc::kUpllUcpCreate)?UNC_OP_CREATE:
    ((phase == uuc::kUpllUcpUpdate)?UNC_OP_UPDATE:
     ((phase == uuc::kUpllUcpDelete)?UNC_OP_DELETE:UNC_OP_INVALID));
  if (phase == uuc::kUpllUcpDelete2)
    return result_code;
  /* retreives the delta of running and audit configuration */
  UPLL_LOG_TRACE("Operation is %d", op);
  if ((op == UNC_OP_UPDATE) && (keytype == UNC_KT_VTN))
    return UPLL_RC_SUCCESS;
  result_code = DiffConfigDB(UPLL_DT_RUNNING, UPLL_DT_AUDIT, op,
      ckv_running, ckv_audit,
      &cursor, dmi, in_ctrlr, tbl, true);
  if (UPLL_RC_SUCCESS != result_code) {
    UPLL_LOG_DEBUG("DiffConfigDB failed - %d", result_code);
    return result_code;
  }
  while (uud::kDalRcSuccess == (db_result = dmi->GetNextRecord(cursor))) {
    /* ignore records of another controller for create and update operation */
    if (phase != uuc::kUpllUcpDelete) {
      uint8_t *db_ctrlr = NULL;
      GET_USER_DATA_CTRLR(ckv_running, db_ctrlr);
      UPLL_LOG_TRACE("db ctrl_id and audit ctlr_id are  %s %s", db_ctrlr, ctrlr_id);
      if (db_ctrlr && strncmp(reinterpret_cast<const char *>(db_ctrlr),
            reinterpret_cast<const char *>(ctrlr_id),
            strlen(reinterpret_cast<const char *>(ctrlr_id)) + 1))
        continue;
    }
    UPLL_LOG_TRACE("Diff Running Record for Keytype: Operation:  is %d %d\n %s",
        keytype, op, ckv_running->ToStrAll().c_str());
    if (op == UNC_OP_UPDATE)
      UPLL_LOG_TRACE("Diff Audit Record for Keytype: Operation:  is %d %d\n %s",
          keytype, op, ckv_audit->ToStrAll().c_str());
    result_code =  GetDiffRecord(ckv_running, ckv_audit, phase, tbl,
        ckv_drvr, dmi, invalid_attr);
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
      continue;
    }
    if (OVERLAY_KT(keytype) && (op == UNC_OP_DELETE)) {
      UPLL_LOG_TRACE("Overlay KeyType %d", keytype);
      DbSubOp dbop = {kOpReadSingle, kOpMatchNone,
        kOpInOutCtrlr | kOpInOutDomain};
      result_code = ReadConfigDB(ckv_drvr, UPLL_DT_AUDIT,
          UNC_OP_READ, dbop, dmi, MAINTBL);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("ReadConfigDB from AuditdB failed %d", result_code);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        return UPLL_RC_ERR_GENERIC;
      }
      if (keytype == UNC_KT_VTUNNEL || keytype == UNC_KT_VTUNNEL_IF) {
        result_code = PopulateDriverDeleteCkv(ckv_drvr, dmi, UPLL_DT_AUDIT);
        if (result_code != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("Couldnt form Driver Val structure %d", result_code);
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          dmi->CloseCursor(cursor, true);
          return UPLL_RC_ERR_GENERIC;
        }
      }
    }
    /* Boundary VLink is not sent to Controller*/
    if (ckv_drvr->get_key_type() == UNC_KT_VLINK && (op != UNC_OP_DELETE)) {
      bool bound_vlink = false;
      VlinkMoMgr *mgr = reinterpret_cast<VlinkMoMgr *>(
          const_cast<MoManager *>(GetMoManager(UNC_KT_VLINK)));
      if (!mgr) {
        UPLL_LOG_DEBUG("Invalid mgr");
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        return UPLL_RC_ERR_GENERIC;
      }
      result_code = mgr->BoundaryVlink(ckv_drvr, vlink_ctrlr_dom, bound_vlink);
      if (bound_vlink) {
        UPLL_LOG_TRACE("%s is a boundary Link. Not Sent to Controller/n",
            (reinterpret_cast<key_vlink *>(ckv_drvr->get_key()))->vlink_name);
        if (ckv_drvr) delete ckv_drvr;
        ckv_drvr = NULL;
        continue;
      }
    }
    GET_USER_DATA_CTRLR_DOMAIN(ckv_drvr, ctrlr_dom);
    UPLL_LOG_DEBUG("Outside if %d", keytype);
    if (!OVERLAY_KT(keytype)) {
      UPLL_LOG_DEBUG("Inside if %d", keytype);
      upll_keytype_datatype_t dt_type = (op == UNC_OP_DELETE)?
        UPLL_DT_AUDIT : UPLL_DT_RUNNING;
      if (ckv_drvr->get_key_type() == UNC_KT_VLINK) {
        if (ckv_drvr->get_cfg_val()) {
          GET_USER_DATA_CTRLR_DOMAIN(ckv_drvr->get_cfg_val(), vlink_ctrlr_dom[1]);
        } else {
          vlink_ctrlr_dom[1].ctrlr = NULL;
          vlink_ctrlr_dom[1].domain = NULL;
        }
      }
      controller_domain_t *tctrlr_dom =
        (ckv_drvr->get_key_type() == UNC_KT_VLINK)?
        &vlink_ctrlr_dom[1]:&ctrlr_dom;
      if (UNC_KT_POLICING_PROFILE == ckv_drvr->get_key_type() ||
          UNC_KT_POLICING_PROFILE_ENTRY == ckv_drvr->get_key_type() ||
          UNC_KT_FLOWLIST == ckv_drvr->get_key_type() ||
          UNC_KT_FLOWLIST_ENTRY == ckv_drvr->get_key_type() ||
          UNC_KT_VTN_POLICINGMAP == ckv_drvr->get_key_type() ||
          UNC_KT_VTN_FLOWFILTER == ckv_drvr->get_key_type() ||
          UNC_KT_VTN_FLOWFILTER_ENTRY == ckv_drvr->get_key_type()) {
        dt_type = UPLL_DT_RUNNING;
      }
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
      result_code = GetRenamedControllerKey(ckv_drvr, dt_type,
          dmi, tctrlr_dom);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG(" GetRenamedControllerKey failed err code(%d)",
            result_code);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
      if (ckv_drvr->get_key_type() == UNC_KT_VLINK && (op == UNC_OP_DELETE)) {
        /* boundary vlink */
        VlinkMoMgr *vlink_mgr = reinterpret_cast<VlinkMoMgr *>(
            const_cast<MoManager *>(GetMoManager(UNC_KT_VLINK)));
        if (!vlink_mgr) {
          UPLL_LOG_DEBUG("Invalid mgr");
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(ckv_drvr);
          dmi->CloseCursor(cursor, true);
          return UPLL_RC_ERR_GENERIC;
        }
        unc_key_type_t ktype[2] = {UNC_KT_ROOT, UNC_KT_ROOT};
        for (int vnode_count = 0; vnode_count < 2; vnode_count++) {
          ktype[vnode_count] = vlink_mgr->
            GetVlinkVnodeIfKeyType(ckv_drvr, vnode_count);
        }
        if (ktype[0] != UNC_KT_VUNK_IF && ktype[1] != UNC_KT_VUNK_IF) {
          if (!ctrlr_dom.ctrlr || !tctrlr_dom->ctrlr || !ctrlr_dom.domain
              || !tctrlr_dom->domain) {
            UPLL_LOG_INFO("Ctrlr id or domain id is Null in vlink delete");
            if (ckv_drvr)
              delete ckv_drvr;
            return UPLL_RC_ERR_GENERIC;
          }
        }
        if (ktype[0] == UNC_KT_VUNK_IF || ktype[1] == UNC_KT_VUNK_IF ||
            strncmp(reinterpret_cast<const char *>(ctrlr_dom.ctrlr), 
              reinterpret_cast<const char *>(tctrlr_dom->ctrlr), 
              kMaxLenCtrlrId + 1) ||
            strncmp(reinterpret_cast<const char *>(ctrlr_dom.domain), 
              reinterpret_cast<const char *>(tctrlr_dom->domain), 
              kMaxLenDomainId + 1)) {
          UPLL_LOG_TRACE("%s is a boundary Link. Not Sent to Controller/n",
              (reinterpret_cast<key_vlink *>(ckv_drvr->get_key()))->vlink_name);
          DELETE_IF_NOT_NULL(resp);
          if (ckv_drvr)
            delete ckv_drvr;
          ckv_drvr = NULL;
          continue;
        }
      }
    }

    if (ctrlr_dom.ctrlr != NULL) {
      bool domain = false;
      KEYTYPE_WITHOUT_DOMAIN(keytype, domain);
      if (!domain) {
        if (NULL == ctrlr_dom.domain) {
          UPLL_LOG_INFO(" domain is NULL");
          DELETE_IF_NOT_NULL(ckv_running);
          DELETE_IF_NOT_NULL(ckv_audit);
          DELETE_IF_NOT_NULL(resp);
          dmi->CloseCursor(cursor, true);
          return UPLL_RC_ERR_GENERIC;
        }
      }
    } else {
      UPLL_LOG_DEBUG("Controller Id is NULL");
      DELETE_IF_NOT_NULL(ckv_running);
      DELETE_IF_NOT_NULL(ckv_audit);
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
    if (!IpcUtil::SendReqToDriver((const char *)ctrlr_dom.ctrlr, reinterpret_cast<char *>
          (ctrlr_dom.domain), PFCDRIVER_SERVICE_NAME,
          PFCDRIVER_SVID_LOGICAL, &ipc_req, true, &ipc_response)) {
      UPLL_LOG_INFO("Request to driver for Key %d for controller %s failed ",
          ckv_drvr->get_key_type(), reinterpret_cast<char *>(ctrlr_dom.ctrlr));
      DELETE_IF_NOT_NULL(resp);
      DELETE_IF_NOT_NULL(ckv_running);
      DELETE_IF_NOT_NULL(ckv_audit);
      DELETE_IF_NOT_NULL(ipc_response.ckv_data);
      dmi->CloseCursor(cursor, true);
      return UPLL_RC_ERR_GENERIC;
    }
    if  ((ipc_response.header.result_code != UPLL_RC_SUCCESS) && (phase != uuc::kUpllUcpDelete)) {
      UPLL_LOG_DEBUG("driver return failure err_code is %d", ipc_response.header.result_code);
      ConfigKeyVal *ctrlr_key = NULL;
      if (CTRLRTBL == (GET_TABLE_TYPE(keytype, tbl))) {
        result_code = DupConfigKeyVal(ctrlr_key, ckv_running, CTRLRTBL);
      } else {
        result_code = DupConfigKeyVal(ctrlr_key, ckv_running, MAINTBL);
      }
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("DupConfigKeyVal failed");
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(resp);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        return result_code;
      }
      if (UNC_KT_VTN == keytype) {
        result_code = UpdateCtrlrConfigStatus(UNC_CS_INVALID,
            phase, ctrlr_key);
      } else {
          result_code = UpdateAuditConfigStatus(UNC_CS_INVALID,
              phase, ctrlr_key);
      }
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("UpdateAuditConfigStatus failed");
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(resp);
        DELETE_IF_NOT_NULL(ckv_running);
        DELETE_IF_NOT_NULL(ckv_audit);
        DELETE_IF_NOT_NULL(ckv_drvr);
        dmi->CloseCursor(cursor, true);
        DELETE_IF_NOT_NULL(ctrlr_key);
        return result_code;
      }
      result_code = UpdateConfigDB(ctrlr_key, UPLL_DT_RUNNING, UNC_OP_UPDATE,
          dmi, tbl);
      if (result_code != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("UpdateConfigDB failed for ipc response ckv err_code %d",
            result_code);
        DELETE_IF_NOT_NULL(ipc_response.ckv_data);
        DELETE_IF_NOT_NULL(resp);
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
          DELETE_IF_NOT_NULL(resp);
          return result_code;
        }
      }
      DELETE_IF_NOT_NULL(ipc_response.ckv_data);
      DELETE_IF_NOT_NULL(ckv_running);
      DELETE_IF_NOT_NULL(ckv_audit);
      DELETE_IF_NOT_NULL(ckv_drvr);
      dmi->CloseCursor(cursor, true);
      DELETE_IF_NOT_NULL(resp);
      return ipc_response.header.result_code; 
    }
    DELETE_IF_NOT_NULL(ckv_drvr);
    DELETE_IF_NOT_NULL(ipc_response.ckv_data);
    *ctrlr_affected = true;
    DELETE_IF_NOT_NULL(resp);
  }
  if (cursor)
    dmi->CloseCursor(cursor, true);
  if (uud::kDalRcSuccess != db_result) {
    UPLL_LOG_DEBUG("GetNextRecord from database failed  - %d", db_result);
    result_code =  DalToUpllResCode(db_result);
  }
  if (ckv_running)
    delete ckv_running;
  if (ckv_audit)
    delete ckv_audit;
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
  UPLL_LOG_INFO("controller id & vote result  is %s %d", ctrlr_id,
      ctrlr_result);
  switch (ctrlr_result) {
    case UPLL_RC_SUCCESS: /* No Operation */
      break;
    case UPLL_RC_ERR_RESOURCE_DISCONNECTED:
      break;
    default:
      /* retrieves the error configkeyval one by one and if the keytype matches,
         rename the key with UNC name */
      ConfigKeyVal *ckv_drv_rslt  = NULL;
      for (ckv_drv_rslt  = vote_status->err_ckv;
          ckv_drv_rslt != NULL; ckv_drv_rslt = ckv_drv_rslt->get_next_cfg_key_val()) {
        if (ckv_drv_rslt->get_key_type() != keytype)
          continue;
        UPLL_LOG_INFO("err ConfigKeyVal are %s", (vote_status->err_ckv)->ToStrAll().c_str());
        /* Get the Unc key */
        if (!OVERLAY_KT(keytype)) {
          result_code = GetRenamedUncKey(ckv_drv_rslt, UPLL_DT_RUNNING,
              dmi, ctrlr_id);
          if (UPLL_RC_SUCCESS != result_code &&
              UPLL_RC_ERR_NO_SUCH_INSTANCE != result_code) {
            UPLL_LOG_DEBUG("GetRenamedUncKey failed - %d", result_code);
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
        DbSubOp dbop = {kOpReadSingle, kOpMatchNone, kOpInOutNone};
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
          DELETE_IF_NOT_NULL(ckv_dup);//check with Vinoth
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
              kOpInOutCtrlr | kOpInOutDomain};
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
                    operation, temp_ctrlr_ckv);
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
                UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
                DELETE_IF_NOT_NULL(ckv_dup);
                DELETE_IF_NOT_NULL(temp_ckv_dup);
                DELETE_IF_NOT_NULL(ctrlr_ckv);
                return result_code;
              }
              temp_ctrlr_ckv = temp_ctrlr_ckv->get_next_cfg_key_val();
            }
            DELETE_IF_NOT_NULL(ctrlr_ckv);
          } else {
            result_code = DupConfigKeyVal(temp_ckv_dup, ckv_drv_rslt, MAINTBL);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("DupConfigKeyVal failed %d", result_code);
              DELETE_IF_NOT_NULL(ckv_dup);
              return result_code;
            }
            result_code = UpdateAuditConfigStatus(UNC_CS_INVALID,
                operation, temp_ckv_dup);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("UpdateAuditConfigStatus failed %d", result_code);
              DELETE_IF_NOT_NULL(ckv_dup);
              DELETE_IF_NOT_NULL(temp_ckv_dup);
              return result_code;
            }
            result_code = UpdateConfigDB(temp_ckv_dup, UPLL_DT_RUNNING,
                UNC_OP_UPDATE, dmi, MAINTBL);
            if (UPLL_RC_SUCCESS != result_code) {
              UPLL_LOG_DEBUG("UpdateConfigDB failed %d", result_code);
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
  UPLL_LOG_INFO("controller id & commit result  is %s %d", ctrlr_id,
                                                   ctrlr_result);
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
        result_code = DiffConfigDB(UPLL_DT_RUNNING, UPLL_DT_AUDIT,
                              operation,
                              ckv_running, ckv_audit,
                              &cursor,
                              dmi, ctrlr_id,  tbl, true);
        if (UPLL_RC_SUCCESS != result_code) {
          UPLL_LOG_DEBUG("DiffConfigDB failed - %d", result_code);
          return result_code;
        }
        /* Get the record one by one, duplicate the configkeyval
         and update cs status */
        while (uud::kDalRcSuccess == (db_result = dmi->GetNextRecord(cursor))) {
          /* ignore records of another controller for create and update operation */
          uint8_t *ctrlr = NULL;
          GET_USER_DATA_CTRLR(ckv_running, ctrlr);
          if (ctrlr && strncmp(reinterpret_cast<const char *>(ctrlr),
                           reinterpret_cast<const char *>(ctrlr_id),
                           sizeof(strlen(reinterpret_cast<const char *>(ctrlr_id)))))
            continue;
          UPLL_LOG_TRACE("Diff Record: Keytype: Phase:  is %d\n %d\n %s",
                    keytype, loop, ckv_running->ToStrAll().c_str());
          result_code =  GetDiffRecord(ckv_running, ckv_audit, (uuc::UpdateCtrlrPhase)loop, tbl,
                                       ckv_update, dmi, invalid_attr);
          if (result_code != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("GetDiffRecord failed err code is %d", result_code);
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
                              (uuc::UpdateCtrlrPhase)loop, ctrlr_key);
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
    case UPLL_RC_ERR_RESOURCE_DISCONNECTED:
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
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  DalResultCode db_result;
  uudst::kDalTableIndex tbl_index;
  for (int tbl = MAINTBL; tbl < ntable; tbl++) {
    tbl_index = GetTable((MoMgrTables)tbl, UPLL_DT_AUDIT);
    UPLL_LOG_DEBUG("Table Index value is %d", tbl_index);
    if (tbl_index >= uudst::kDalNumTables)
      continue;
    db_result = dmi->DeleteRecords(UPLL_DT_AUDIT, tbl_index, NULL);
    result_code = DalToUpllResCode(db_result);
    result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE)
                ? UPLL_RC_SUCCESS : result_code;
    if (UPLL_RC_SUCCESS != result_code) {
        UPLL_LOG_DEBUG("DAL error in DeleteRecords call  - %d", result_code);
        break;
    }
  }
  return result_code;
}

upll_rc_t MoMgrImpl::ClearStartup(unc_key_type_t kt,
                                  DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    upll_rc_t result_code = UPLL_RC_SUCCESS;
    DalResultCode db_result;
    for (int tbl = MAINTBL; tbl < ntable; tbl++) {
      uudst::kDalTableIndex tbl_index;
      tbl_index = GetTable((MoMgrTables)tbl, UPLL_DT_IMPORT);
      if ((tbl_index >= uudst::kDalNumTables) ||
        ((tbl == RENAMETBL) && (kt != UNC_KT_VBRIDGE)))
        continue;
      db_result =  dmi->DeleteRecords(UPLL_DT_STARTUP, tbl_index, NULL);
      result_code = DalToUpllResCode(db_result);
      if (result_code != UPLL_RC_SUCCESS &&
          result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE)
        return result_code;
    }
    return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::CopyRunningToStartup(unc_key_type_t kt,
                                          DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;

  upll_keytype_datatype_t dest_cfg_type = UPLL_DT_STARTUP;
  upll_keytype_datatype_t src_cfg_type = UPLL_DT_RUNNING;

  for (int i = MAINTBL; i < ntable; i++)  {
    DalResultCode db_result  = uud::kDalRcSuccess;
    const uudst::kDalTableIndex tbl_index = GetTable((MoMgrTables)i,
                                                     src_cfg_type);
    if (tbl_index >= uudst::kDalNumTables)
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

upll_rc_t MoMgrImpl::CopyStartupToCandidate(unc_key_type_t kt,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_keytype_datatype_t dest_cfg_type = UPLL_DT_CANDIDATE;
  upll_keytype_datatype_t src_cfg_type = UPLL_DT_STARTUP;
  for (int i = MAINTBL; i < ntable; i++)  {
    DalResultCode db_result  = uud::kDalRcSuccess;
    const uudst::kDalTableIndex tbl_index = GetTable((MoMgrTables)i,
                                                     src_cfg_type);
    if (tbl_index >= uudst::kDalNumTables)
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

upll_rc_t MoMgrImpl::CopyRunningToCandidate(unc_key_type_t kt,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  DalResultCode db_result = uud::kDalRcSuccess;
  upll_keytype_datatype_t dest_cfg_type = UPLL_DT_CANDIDATE;
  upll_keytype_datatype_t src_cfg_type = UPLL_DT_RUNNING;
  for (int i = MAINTBL; i < ntable; i++)  {
    const uudst::kDalTableIndex tbl_index = GetTable((MoMgrTables)i,
                                                     src_cfg_type);
    if (tbl_index >= uudst::kDalNumTables)
      continue;
    db_result = dmi->CopyModifiedRecords(dest_cfg_type, src_cfg_type, tbl_index,
                  NULL);
    if (db_result != uud::kDalRcSuccess) {
      break;
    }
  }
  return DalToUpllResCode(db_result);
}

upll_rc_t MoMgrImpl::LoadStartup(unc_key_type_t kt, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  /* copy startup to candidate */
  upll_rc_t result_code = CopyStartupToCandidate(kt, dmi);
  if (result_code != UPLL_RC_SUCCESS)
    return result_code;

  /* copy startup to running */
  upll_keytype_datatype_t dest_cfg_type = UPLL_DT_RUNNING;
  upll_keytype_datatype_t src_cfg_type = UPLL_DT_STARTUP;
  for (int tbl = MAINTBL; tbl < ntable; tbl++) {
    DalResultCode db_result  = uud::kDalRcSuccess;
    const uudst::kDalTableIndex tbl_index = GetTable((MoMgrTables)tbl,
                                                     dest_cfg_type);
    if (tbl_index >= uudst::kDalNumTables)
      continue;
    db_result = dmi->CopyEntireRecords(dest_cfg_type, src_cfg_type,
                                     tbl_index, NULL);
    if ((db_result != uud::kDalRcSuccess) &&
        (db_result != uud::kDalRcRecordNotFound)) {
      UPLL_LOG_INFO("Copy error");
      break;
    }
    if (tbl != RENAMETBL) {
      DalBindInfo dal_bind_info(tbl_index);
      result_code = BindStartup(&dal_bind_info, dest_cfg_type,
                                           (MoMgrTables)tbl);
      if (result_code != UPLL_RC_SUCCESS) {
        return result_code;
      }
      db_result = dmi->UpdateRecords(dest_cfg_type, tbl_index,
                                    &dal_bind_info);
      if ((db_result != uud::kDalRcSuccess) &&
          (db_result != uud::kDalRcRecordNotFound)) {
        UPLL_LOG_INFO("Update error %d, kt - %d", db_result, kt);
        return DalToUpllResCode(db_result);
      }
    }
  }
  return UPLL_RC_SUCCESS;
}

upll_rc_t MoMgrImpl::IsCandidateDirty(unc_key_type_t kt,
                                      bool *dirty,
                                      DalDmlIntf *dmi) {
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
    DalBindInfo dal_bind_info(tbl_index);
    result_code = BindCandidateDirty(&dal_bind_info,
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
      return UPLL_RC_ERR_RESOURCE_DISCONNECTED;
    case uud::kDalRcTxnError:
      return UPLL_RC_ERR_GENERIC;
    case uud::kDalRcInvalidConnHandle:
      return UPLL_RC_ERR_DB_ACCESS;
    case uud::kDalRcRecordNotFound:
    case uud::kDalRcRecordNoMore:
      return UPLL_RC_ERR_NO_SUCH_INSTANCE;
    case uud::kDalRcRecordAlreadyExists:
      return UPLL_RC_ERR_INSTANCE_EXISTS;
    case uud::kDalRcAccessViolation:
      return UPLL_RC_ERR_DB_ACCESS;
    case uud::kDalRcConnNotEstablished:
    case uud::kDalRcDataError:
    case uud::kDalRcInvalidCursor:
    case uud::kDalRcMemoryError:
    case uud::kDalRcInternalError:
    case uud::kDalRcGeneralError:
    default:
      return UPLL_RC_ERR_GENERIC;
  }
}



upll_rc_t MoMgrImpl:: UpdateVnodeTables(ConfigKeyVal *ikey,
                                         upll_keytype_datatype_t data_type,
                                         DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (!ikey) {
    UPLL_LOG_DEBUG("Given Input is Empty ");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  for (int tbl = MAINTBL; tbl < ntable; tbl++) {
    if (GetTable((MoMgrTables)tbl, data_type) < uudst::kDalNumTables) {
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
    result_code = ReadConfigDB(okey, data_type, UNC_OP_READ, dbop, dmi, MAINTBL);
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
          default:
            break;
        }
        UPLL_LOG_TRACE("The Renamed Value is %d", renamed);
        if (!renamed && !no_rename) {
          /* If VTN is renamed add the entry in vnode and vlink rename table */
          if (table[MAINTBL]->get_key_type() == UNC_KT_VBRIDGE ||
              table[MAINTBL]->get_key_type() == UNC_KT_VROUTER ||
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
      if (OVERLAY_KT(ktype)) {
        UPLL_LOG_TRACE("Overlay Key type %d is skipping from rename operation"
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
          UNC_KT_VRTIF_FLOWFILTER_ENTRY };

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
          UNC_KT_VRTIF_FLOWFILTER, UNC_KT_VRTIF_FLOWFILTER_ENTRY};
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
          UNC_KT_VBRIF_POLICINGMAP };
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
  unc_key_type_t nodes[] = {UNC_KT_VBRIDGE, UNC_KT_VROUTER, UNC_KT_VLINK,
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
  result_code = (result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) ? UPLL_RC_SUCCESS :
                                      result_code;
  return result_code;
}

unc_keytype_configstatus_t MoMgrImpl::GetConsolidatedCsStatus(
                             list< unc_keytype_configstatus_t > cs_status) {
  unc_keytype_configstatus_t final_cs_status;
  unc_keytype_configstatus_t current_cs_status;

  list< unc_keytype_configstatus_t >::iterator iter;
  iter = cs_status.begin();
  final_cs_status = *iter;
  if (iter != cs_status.end() && UNC_CS_NOT_SUPPORTED == final_cs_status) {
    return UNC_CS_NOT_APPLIED;
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
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc



