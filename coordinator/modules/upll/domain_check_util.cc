/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "pfc/ipc_struct.h"
#include "unc/usess_ipc.h"
#include "uncxx/upll_log.hh"
#include "momgr_impl.hh"
#include "dbconn_mgr.hh"
#include "ipc_util.hh"
#include "ipct_st.hh"
#include "domain_check_util.hh"
#include "upll_validation.hh"

namespace unc {
namespace upll {
namespace domain_util {

using unc::upll::dal::DalBindInfo;
using unc::upll::dal::DalColumnIndex;
using unc::upll::dal::DalResultCode;
using unc::upll::ipc_util::ConfigKeyVal;
using unc::upll::ipc_util::ConfigVal;
using unc::upll::ipc_util::IpcResponse;
using unc::upll::ipc_util::IpcReqRespHeader;
using unc::upll::ipc_util::IpcRequest;
using unc::upll::ipc_util::IpctSt;

namespace uud = unc::upll::dal;
namespace uuu = unc::upll::upll_util;
namespace uui = unc::upll::ipc_util;
namespace uudst = unc::upll::dal::schema::table;

// Here tbl_idx supports only spine domain table and vbr port map table.
upll_rc_t DomainUtil::IsDomainExistInTable(
    const char* ctrlr_name, const char* domain_name, DalDmlIntf *dmi,
    upll_keytype_datatype_t dt_type, kDalTableIndex tbl_idx) {
  UPLL_FUNC_TRACE;
  DalColumnIndex dcidx1 = 0, dcidx2 = 0;
  if (tbl_idx == uudst::kDbiUnwSpineDomainTbl) {
    dcidx1 = uudst::unw_spine_domain::kDbiCtrlrName;
    dcidx2 = uudst::unw_spine_domain::kDbiDomainId;
  } else if (tbl_idx == uudst::kDbiVbrPortMapTbl) {
    dcidx1 = uudst::vbridge_portmap::kDbiCtrlrName;
    dcidx2 = uudst::vbridge_portmap::kDbiDomainId;
  } else {
    UPLL_LOG_ERROR("Table index :%u is not supported", tbl_idx);
  }
  DalBindInfo bind_info(tbl_idx);
  bind_info.BindMatch(dcidx1, uud::kDalChar,
                      unc::upll::kt_momgr::kMaxLenCtrlrId+1, ctrlr_name);
  bind_info.BindMatch(dcidx2, uud::kDalChar,
                      unc::upll::kt_momgr::kMaxLenDomainId+1, domain_name);
  bool existence;
  DalResultCode dal_rc;
  dal_rc = dmi->RecordExists(dt_type, tbl_idx, &bind_info, &existence);
  upll_rc_t urc =
        unc::upll::config_momgr::UpllDbConnMgr::ConvertDalResultCode(dal_rc);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("RecordExists failed, %d", urc);
    return urc;
  }
  if (!existence) {
    UPLL_LOG_DEBUG("<%s,%s> not in DT %d", ctrlr_name, domain_name, dt_type);
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  UPLL_LOG_TRACE("<%s,%s> exists in DT %d", ctrlr_name, domain_name, dt_type);
  return urc;
}

upll_rc_t DomainUtil::GetDomainTypeFromPhysical(
     const char* ctrlr_name, const char* domain_name,
     UpplDomainType *dom_type) {
  UPLL_FUNC_TRACE;

  upll_rc_t urc = UPLL_RC_SUCCESS;

  key_ctr_domain *ctr_dom_key = ConfigKeyVal::Malloc<key_ctr_domain>();
  uuu::upll_strncpy(ctr_dom_key->ctr_key.controller_name, ctrlr_name,
                    unc::upll::kt_momgr::kMaxLenCtrlrId+1);
  uuu::upll_strncpy(ctr_dom_key->domain_name, domain_name,
                    unc::upll::kt_momgr::kMaxLenDomainId+1);
  ConfigKeyVal ckv_ctr_dom(UNC_KT_CTR_DOMAIN, IpctSt::kIpcStKeyCtrDomain,
                           ctr_dom_key, NULL);

  IpcRequest ipc_req;
  memset(&ipc_req, 0, sizeof(ipc_req));
  ipc_req.header.clnt_sess_id = USESS_ID_UPLL;
  ipc_req.header.config_id = 0;
  ipc_req.header.operation = UNC_OP_READ;
  ipc_req.header.datatype = UPLL_DT_STATE;
  ipc_req.ckv_data = &ckv_ctr_dom;

  IpcResponse ipc_resp;
  memset(&ipc_resp, 0, sizeof(IpcResponse));

  if (!uui::IpcUtil::SendReqToPhysical(UPPL_IPC_SVC_NAME, UPPL_SVC_READREQ,
                                       &ipc_req, &ipc_resp)) {
      UPLL_LOG_INFO("Send Request to physical for UNC_KT_CTR_DOMAIN failed.");
      return UPLL_RC_ERR_GENERIC;
  }

  if (ipc_resp.header.result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UPLL_LOG_DEBUG("Information is not exists in UPPL, skip the validation");
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  } else if (ipc_resp.header.result_code != UPLL_RC_SUCCESS) {
    UPLL_LOG_ERROR("UPPL returns error for READ:%d", urc);
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    urc  = ipc_resp.header.result_code;
    return urc;
  } else {
    if (!ipc_resp.ckv_data) {
      UPLL_LOG_ERROR("ckv_data from UPLL is NULL");
      return UPLL_RC_ERR_GENERIC;
    }
    val_ctr_domain_st *ctr_dom_stval = reinterpret_cast<val_ctr_domain_st*>(
        GetVal(ipc_resp.ckv_data));

    if (!ctr_dom_stval) {
      UPLL_LOG_ERROR("val structure from UPPL is NULL");
      DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
      return UPLL_RC_ERR_GENERIC;
    }
    if (ctr_dom_stval->valid[kIdxDomainStDomain] == UNC_VF_VALID) {
      if (ctr_dom_stval->domain.valid[kIdxDomainType] == UNC_VF_VALID) {
        *dom_type = (UpplDomainType)ctr_dom_stval->domain.type;
      } else {
        urc = UPLL_RC_ERR_NO_SUCH_INSTANCE;
      }
    } else {
      urc = UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    return urc;
  }
}

// Returns SUCCESS, SEMANTIC_ERROR or system errors.
// Returns SUCCESS if no domain is found in VBRPM table or in Physical
upll_rc_t DomainUtil::ValidateSpineDomain(
    const char* ctrlr_name, const char* domain_name,
    DalDmlIntf *dmi, upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = UPLL_RC_SUCCESS;
  urc = IsDomainExistInTable(ctrlr_name, domain_name, dmi, dt_type,
                             uudst::kDbiVbrPortMapTbl);
  if (urc == UPLL_RC_SUCCESS) {
    return UPLL_RC_ERR_CFG_SEMANTIC;
  } else if (urc == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    UpplDomainType dom_type = UPPL_DOMAIN_TYPE_PF_SPINE;
    urc = GetDomainTypeFromPhysical(ctrlr_name, domain_name, &dom_type);
    if (urc == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      urc = UPLL_RC_SUCCESS;
    } else {
      urc = ((dom_type == UPPL_DOMAIN_TYPE_PF_SPINE)?
             UPLL_RC_SUCCESS : UPLL_RC_ERR_CFG_SEMANTIC);
    }
  } else {
    UPLL_LOG_ERROR("DB Err Occured in RecordExists Err:%u", urc);
    return urc;
  }
  return urc;
}

upll_rc_t DomainUtil::IsDomainLeaf(const char* ctrlr_name,
                                   const char* domain_name,
                                   DalDmlIntf *dmi,
                                   upll_keytype_datatype_t dt_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = UPLL_RC_SUCCESS;
  urc = IsDomainExistInTable(ctrlr_name, domain_name, dmi, dt_type,
                                      uudst::kDbiVbrPortMapTbl);
  if (urc == UPLL_RC_SUCCESS) {
    return urc;
  } else if (urc == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
    urc = IsDomainExistInTable(ctrlr_name, domain_name, dmi, dt_type,
                                      uudst::kDbiUnwSpineDomainTbl);
    if (urc == UPLL_RC_SUCCESS) {
      return UPLL_RC_ERR_CFG_SEMANTIC;
    } else if (urc == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      UpplDomainType dom_type = UPPL_DOMAIN_TYPE_PF_LEAF;
      urc = GetDomainTypeFromPhysical(ctrlr_name, domain_name, &dom_type);
      if (urc == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
        urc = UPLL_RC_SUCCESS;
      } else {
        urc = ((dom_type == UPPL_DOMAIN_TYPE_PF_LEAF) ?
               UPLL_RC_SUCCESS : UPLL_RC_ERR_CFG_SEMANTIC);
      }
    } else {
      UPLL_LOG_ERROR("Error occured in RecordExists Err:%u", urc);
      return urc;
    }
  } else {
    UPLL_LOG_ERROR("Error occured in RecordExists Err:%u", urc);
    return urc;
  }
  return urc;
}

}  // namespace domain_util
}  // namespace upll
}  // namespace unc

