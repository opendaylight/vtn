/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include "key_tree.hh"
#include "ipc_util.hh"
#include "ipct_st.hh"
#include "config_mgr.hh"

namespace unc {
namespace upll {
namespace config_momgr {

using unc::upll::ipc_util::IpctSt;

static bool GetNextSiblingKT(const KeyTree &kt_tree, const unc_key_type_t kt,
                             unc_key_type_t *next_sibling_kt,
                             upll_keytype_datatype_t datatype,
                             TcConfigMode cfg_mode) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_TRACE("Request KT = %d, datatype = %d, cfg_mode = %d",
                 kt, datatype, cfg_mode);
  UpllConfigMgr *ucm = UpllConfigMgr::GetUpllConfigMgr();

  bool ok = kt_tree.GetNextSibling(kt, next_sibling_kt);
  if (ok) {
    ok = ucm->IsKtPartOfConfigMode(*next_sibling_kt, datatype, cfg_mode);
  }
  if (ok) {
    if (ucm->GetMoManager(*next_sibling_kt) == NULL) {
      UPLL_LOG_TRACE("Skipping KT = %d", *next_sibling_kt);
      return GetNextSiblingKT(kt_tree, *next_sibling_kt, next_sibling_kt,
                              datatype, cfg_mode);
    } else {
      UPLL_LOG_TRACE("Found KT = %d", *next_sibling_kt);
    }
  }
  return ok;
}

static bool GetFirstChildKT(const KeyTree &kt_tree,
                            unc_key_type_t parent_kt, unc_key_type_t *child_kt,
                            upll_keytype_datatype_t datatype,
                            TcConfigMode cfg_mode) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_TRACE("Request parent KT = %d datatype = %d, cfg_mode = %d",
                 parent_kt, datatype, cfg_mode);
  UpllConfigMgr *ucm = UpllConfigMgr::GetUpllConfigMgr();

  bool ok = kt_tree.GetFirstChild(parent_kt, child_kt);
  if (ok) {
    ok = ucm->IsKtPartOfConfigMode(*child_kt, datatype, cfg_mode);
  }
  if (ok) {
    if (ucm->GetMoManager(*child_kt) == NULL) {
      UPLL_LOG_TRACE("Skipping KT = %d", *child_kt);
      return GetNextSiblingKT(kt_tree, *child_kt, child_kt, datatype, cfg_mode);
    } else {
      UPLL_LOG_TRACE("Found KT = %d", *child_kt);
    }
  }
  return ok;
}

static ConfigKeyVal *GetCkvFromParent(unc_key_type_t desired_kt,
                                      const ConfigKeyVal *in_ckv) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_TRACE("Request sibling KT = %d, in_ckv_kt = %d", desired_kt,
                 in_ckv->get_key_type());
  MoManager *mgr = UpllConfigMgr::GetUpllConfigMgr()->GetMoManager(desired_kt);
  if (mgr == NULL) {
    UPLL_LOG_DEBUG("Shouldn't be here. KeyType=%u", desired_kt);
    return NULL;
  }
  ConfigKeyVal *out_ckv = NULL;
  if (mgr->GetChildConfigKey(out_ckv, const_cast<ConfigKeyVal *>(in_ckv)) ==
      UPLL_RC_SUCCESS) {
    return out_ckv;
  }
  return NULL;
}


static ConfigKeyVal *GetCkvFromSibling(unc_key_type_t desired_kt,
                                       const ConfigKeyVal *in_ckv) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_TRACE("Request sibling KT = %d, in_ckv_kt = %d", desired_kt,
                 in_ckv->get_key_type());
  if (desired_kt == UNC_KT_VTN) {
    return new ConfigKeyVal(UNC_KT_VTN, IpctSt::kIpcStKeyVtn,
                            ConfigKeyVal::Malloc(sizeof(key_vtn)));
  } else if (desired_kt == UNC_KT_FLOWLIST) {
    return new ConfigKeyVal(UNC_KT_FLOWLIST, IpctSt::kIpcStKeyFlowlist,
                            ConfigKeyVal::Malloc(sizeof(key_flowlist)), NULL);
  } else if (desired_kt == UNC_KT_POLICING_PROFILE) {
    return new ConfigKeyVal(UNC_KT_POLICING_PROFILE,
                            IpctSt::kIpcStKeyPolicingprofile,
                            ConfigKeyVal::Malloc(sizeof(key_policingprofile)),
                            NULL);
  }

  MoManager *mgr = UpllConfigMgr::GetUpllConfigMgr()->GetMoManager(
      in_ckv->get_key_type());
  if (mgr == NULL) {
    UPLL_LOG_DEBUG("Shouldn't be here. KeyType=%u", desired_kt);
    return NULL;
  }
  ConfigKeyVal *parent_ckv = NULL;
  if (mgr->GetParentConfigKey(parent_ckv, const_cast<ConfigKeyVal *>(in_ckv)) !=
      UPLL_RC_SUCCESS) {
    return NULL;
  }
  ConfigKeyVal *out_ckv = GetCkvFromParent(desired_kt, parent_ckv);
  delete parent_ckv;
  return out_ckv;
}

static ConfigKeyVal *GetParentCkvFromChild(const ConfigKeyVal *in_ckv) {
  UPLL_FUNC_TRACE;

  UPLL_LOG_TRACE("Request in_ckv_kt = %d", in_ckv->get_key_type());

  MoManager *mgr = UpllConfigMgr::GetUpllConfigMgr()->GetMoManager(
      in_ckv->get_key_type());
  if (mgr == NULL) {
    UPLL_LOG_DEBUG("Shouldn't be here. KeyType=%u", in_ckv->get_key_type());
    return NULL;
  }
  ConfigKeyVal *parent_ckv = NULL;
  if (mgr->GetParentConfigKey(parent_ckv, const_cast<ConfigKeyVal*>(in_ckv)) !=
      UPLL_RC_SUCCESS) {
    UPLL_LOG_TRACE("Failed to get Parent CKV for %u", in_ckv->get_key_type());
    return NULL;
  }
  return parent_ckv;
}

static upll_rc_t ReadBulkGetMo(const IpcReqRespHeader &in_reqhdr,
                               const ConfigKeyVal *req_ckv,
                               ConfigKeyVal **resp_ckv, DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  IpcReqRespHeader msghdr;
  bzero(&msghdr, sizeof(msghdr));
  msghdr.operation = UNC_OP_READ;
  msghdr.datatype = in_reqhdr.datatype;
  msghdr.clnt_sess_id = in_reqhdr.clnt_sess_id;
  msghdr.config_id = in_reqhdr.config_id;

  *resp_ckv = NULL;

  UPLL_LOG_TRACE("Request:%s", req_ckv->ToStr().c_str());

  MoManager *momgr = UpllConfigMgr::GetUpllConfigMgr()->GetMoManager(
      req_ckv->get_key_type());
  if (momgr == NULL) {
    UPLL_LOG_INFO("KT %u is not managed by UPLL", req_ckv->get_key_type());
    return UPLL_RC_ERR_NO_SUCH_NAME;
  }

  // TODO(U17): Do we need to check if the req_ckv falls outside the cfg_mode?

  ConfigKeyVal *mo_ckv = req_ckv->DupKey();
  upll_rc_t urc = momgr->ReadMo(&msghdr, mo_ckv, dmi);
  if (urc != UPLL_RC_SUCCESS) {
    delete mo_ckv;
  } else {
    // (*resp_ckv)->ResetWith(mo_ckv);
    *resp_ckv = mo_ckv;
  }
  UPLL_LOG_TRACE("Req:%s\nResponse: %s", req_ckv->ToStr().c_str(),
                 ((*resp_ckv) ? (*resp_ckv)->ToStrAll().c_str() : "null"));
  return urc;
}

// Get one sibling
static upll_rc_t ReadBulkGetSibling(const IpcReqRespHeader &in_reqhdr,
                                    const ConfigKeyVal *req_ckv,
                                    ConfigKeyVal **resp_ckv,
                                    bool begin,
                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  IpcReqRespHeader msghdr;
  bzero(&msghdr, sizeof(msghdr));
  msghdr.operation = (begin ? UNC_OP_READ_SIBLING_BEGIN : UNC_OP_READ_SIBLING);
  msghdr.rep_count = 1;
  msghdr.datatype = in_reqhdr.datatype;
  msghdr.clnt_sess_id = in_reqhdr.clnt_sess_id;
  msghdr.config_id = in_reqhdr.config_id;

  UPLL_LOG_TRACE("Request:begin=%d, ckv=%s", begin, req_ckv->ToStr().c_str());
  *resp_ckv = NULL;

  if (begin == true) {
    if (req_ckv->get_cfg_val() != NULL) {
      UPLL_LOG_TRACE("Warning: VAL present in READ_SIBLING_BEGIN!");
    }
  }

  MoManager *momgr = UpllConfigMgr::GetUpllConfigMgr()->GetMoManager(
      req_ckv->get_key_type());
  if (momgr == NULL) {
    UPLL_LOG_INFO("KT %u is not managed by UPLL", req_ckv->get_key_type());
    return UPLL_RC_ERR_NO_SUCH_NAME;
  }

  ConfigKeyVal *mo_ckv = req_ckv->DupKey();
  upll_rc_t urc = momgr->ReadSiblingMo(&msghdr, mo_ckv, begin, dmi);
  if (urc != UPLL_RC_SUCCESS) {
    delete mo_ckv;
  } else {
    *resp_ckv = mo_ckv;
    // It is assumed that MoMgrIntf::ReadSiblingMo of KT_VTN never returns KTs
    // outside the view. So no need to do additional checks here for partial
    // configuration mode boundary conditions.
  }
  UPLL_LOG_TRACE("Req:%s\nurc=%d\nResponse: %s", req_ckv->ToStr().c_str(), urc,
                 ((*resp_ckv) ? (*resp_ckv)->ToStrAll().c_str() : "null"));
  return urc;
}

// req_ckv should contain fully specified key for the KT in it.
// returns UPLL_RC_SUCESS if process is done with out err.
// added_cnt will be zero if no child is found for the given instance
upll_rc_t UpllConfigMgr::ReadBulkGetSubtree(const KeyTree &kt_tree,
                                            const IpcReqRespHeader &in_reqhdr,
                                            TcConfigMode cfg_mode,
                                            const ConfigKeyVal *user_req_ckv,
                                            uint32_t requested_cnt,
                                            ConfigKeyVal **user_resp_ckv,
                                            uint32_t *added_cnt,
                                            DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_TRACE("Request:%s", user_req_ckv->ToStr().c_str());
  *added_cnt = 0;
  if (requested_cnt == 0) {
    return UPLL_RC_SUCCESS;
  }

  *user_resp_ckv = NULL;

  unc_key_type_t curr_kt = user_req_ckv->get_key_type();
  unc_key_type_t child_kt;
  bool keytree_ret;
  // For all children KT
  for ((keytree_ret = GetFirstChildKT(kt_tree, curr_kt, &child_kt,
                                      in_reqhdr.datatype, cfg_mode));
       keytree_ret;
       (keytree_ret = GetNextSiblingKT(kt_tree, curr_kt, &child_kt,
                                       in_reqhdr.datatype, cfg_mode))) {
    curr_kt = child_kt;
    UPLL_LOG_DEBUG("Child KT %u", curr_kt);
    ConfigKeyVal *curr_req_ckv = GetCkvFromParent(curr_kt, user_req_ckv);
    if (curr_req_ckv == NULL) {
      UPLL_LOG_INFO("Could not initialize CKV for %u", curr_kt);
      if (*user_resp_ckv != NULL) {
        delete *user_resp_ckv;
        *user_resp_ckv = NULL;
      }
      return UPLL_RC_ERR_GENERIC;
    }
    upll_rc_t curr_urc;
    ConfigKeyVal *curr_resp_ckv = NULL;
    for ((curr_urc = ReadBulkGetSibling(in_reqhdr, curr_req_ckv, &curr_resp_ckv,
                                        true, dmi));
         curr_urc == UPLL_RC_SUCCESS;
         (curr_urc = ReadBulkGetSibling(in_reqhdr, curr_req_ckv, &curr_resp_ckv,
                                        false, dmi))) {
      delete curr_req_ckv;
      curr_req_ckv = curr_resp_ckv->DupKey();  // prepare for loop
      if (curr_req_ckv == NULL) {
        if (*user_resp_ckv != NULL) {
          UPLL_LOG_INFO("DupKey failed for %u", curr_resp_ckv->get_key_type());
          delete *user_resp_ckv;
          *user_resp_ckv = NULL;
        }
        return UPLL_RC_ERR_GENERIC;
      }

      if (*user_resp_ckv == NULL) {
        *user_resp_ckv = curr_resp_ckv;   // init user_resp_ckv
      } else {
        (*user_resp_ckv)->AppendCfgKeyVal(curr_resp_ckv);
      }

      (*added_cnt)++;
      if ((*added_cnt) == requested_cnt) {
        delete curr_req_ckv;
        return UPLL_RC_SUCCESS;
      }

      uint32_t new_cnt = 0;
      ConfigKeyVal *local_bulk_ckv_req = curr_resp_ckv->DupKey();
      ConfigKeyVal *local_bulk_ckv_resp = NULL;
      upll_rc_t new_urc = ReadBulkGetSubtree(kt_tree, in_reqhdr, cfg_mode,
                                             local_bulk_ckv_req,
                                             (requested_cnt - (*added_cnt)),
                                             &local_bulk_ckv_resp, &new_cnt,
                                             dmi);
      delete local_bulk_ckv_req;
      if (new_urc == UPLL_RC_SUCCESS) {
        (*added_cnt) += new_cnt;
        (*user_resp_ckv)->AppendCfgKeyVal(local_bulk_ckv_resp);
        if ((*added_cnt) == requested_cnt) {
          delete curr_req_ckv;
          return UPLL_RC_SUCCESS;
        }
      } else {
        if (*user_resp_ckv != NULL) {
          delete *user_resp_ckv;
          *user_resp_ckv = NULL;
        }
        delete curr_req_ckv;
        return new_urc;
      }
      UPLL_LOG_DEBUG("KT: %u Got %u Urc %d", curr_kt, (*added_cnt), new_urc);
    }  // For all sibling instances

    delete curr_req_ckv;

    if ((curr_urc != UPLL_RC_ERR_NO_SUCH_INSTANCE) &&
        (curr_urc != UPLL_RC_SUCCESS)) {
      if (*user_resp_ckv != NULL) {
        delete *user_resp_ckv;
        *user_resp_ckv = NULL;
      }
      return curr_urc;
    }
  }  // For all children KT

  UPLL_LOG_DEBUG("KT: %u Requested: %u, Got %u",
                 user_req_ckv->get_key_type(), requested_cnt, (*added_cnt));
  UPLL_LOG_TRACE("Req:%s\nResponse: %s", user_req_ckv->ToStr().c_str(),
                 ((*user_resp_ckv) ? (*user_resp_ckv)->ToStrAll().c_str()
                  : "null"));
  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllConfigMgr::ReadBulkMo(IpcReqRespHeader *msghdr,
                                    const ConfigKeyVal *user_req_ckv,
                                    ConfigKeyVal **user_resp_ckv,
                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  if (msghdr == NULL || user_req_ckv == NULL)
    return UPLL_RC_ERR_GENERIC;
  *user_resp_ckv = NULL;

  // In READ_BULK request, value is not expected
  if (user_req_ckv->get_cfg_val() != NULL) {
    msghdr->result_code = UPLL_RC_ERR_BAD_REQUEST;
    msghdr->rep_count = 0;
    return (upll_rc_t)msghdr->result_code;
  }

  TcConfigMode cfg_mode;
  if (msghdr->datatype == UPLL_DT_CANDIDATE) {
    upll_rc_t tc_rc = GetConfigMode(msghdr->clnt_sess_id, msghdr->config_id,
                                    &cfg_mode);
    if (tc_rc != UPLL_RC_SUCCESS) {
      return tc_rc;
    }
  } else {
    cfg_mode = TC_CONFIG_GLOBAL;
  }

  uint32_t pending_cnt = msghdr->rep_count;
  msghdr->rep_count = 0;
  if (pending_cnt == 0) {
    msghdr->result_code = UPLL_RC_SUCCESS;
    return UPLL_RC_SUCCESS;
  } else if (pending_cnt > MoManager::kMaxReadBulkCount) {
    pending_cnt = MoManager::kMaxReadBulkCount;
  }

  KeyTree *kt_tree = NULL;
  switch (msghdr->datatype) {
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_RUNNING:
    case UPLL_DT_STARTUP:
      kt_tree = &cktt_;
      break;
    case UPLL_DT_IMPORT:
      kt_tree = &iktt_;
      break;
    default:
      msghdr->result_code = UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
      return msghdr->result_code;
  }

  // READ lock on given datatype is already taken in TakeConfigLock
  // so no lock required here

  uint32_t added_cnt = 0;

  if (user_req_ckv->get_key_type() == UNC_KT_ROOT) {
    // TODO(U17): Is KT_ROOT accessible from VTN mode? I believe so for
    // READ_NEXT or READ_BULK
    upll_rc_t urc = ReadBulkGetSubtree(*kt_tree, *msghdr, cfg_mode,
                                       user_req_ckv,
                                       pending_cnt, user_resp_ckv,
                                       &added_cnt, dmi);
    if (urc == UPLL_RC_SUCCESS) {
      if (added_cnt == 0) {
        msghdr->result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
      } else {
        msghdr->result_code = UPLL_RC_SUCCESS;
        msghdr->rep_count = added_cnt;
      }
    } else {
      msghdr->result_code = urc;
    }
    UPLL_LOG_TRACE("Returning from %s", __FUNCTION__);
    return urc;
  }

  // Find first instance in the database
  MoManager *momgr = GetMoManager(user_req_ckv->get_key_type());
  if (momgr == NULL) {
    UPLL_LOG_DEBUG("KT %u is not managed by UPLL",
                   user_req_ckv->get_key_type());
    msghdr->result_code = UPLL_RC_ERR_NO_SUCH_NAME;
    return UPLL_RC_ERR_NO_SUCH_NAME;
  }

  bool begin = true;
  bool retrieve_user_req_mo = true;  // Check the presence of user given key
  const ConfigKeyVal *step_req_ckv = user_req_ckv->DupKey();
  if (!step_req_ckv) {
    UPLL_LOG_DEBUG("step_req_ckv is NULL");
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t urc;
  while (pending_cnt > 0) {
    ConfigKeyVal *step_resp_ckv = NULL;
    if (retrieve_user_req_mo) {   // execute only once
      urc = ReadBulkGetMo(*msghdr, step_req_ckv, &step_resp_ckv, dmi);
      switch (urc) {
        case UPLL_RC_ERR_CFG_SYNTAX:
          {
            UPLL_LOG_TRACE("User given key has syntax error. Key = %s",
                           step_req_ckv->ToStr().c_str());
            // probably the key is partial, try sibling begin
            retrieve_user_req_mo = false;
            urc = ReadBulkGetSibling(*msghdr, step_req_ckv,
                                     &step_resp_ckv, true, dmi);
          }
          break;
        case UPLL_RC_ERR_NO_SUCH_INSTANCE:
          {
            UPLL_LOG_TRACE("User given key does not exist. Key = %s",
                           step_req_ckv->ToStr().c_str());
            retrieve_user_req_mo = false;
            urc = ReadBulkGetSibling(*msghdr, step_req_ckv,
                                     &step_resp_ckv, false, dmi);
          }
          break;
        case UPLL_RC_SUCCESS:
          begin = false;
          break;
        default:
          UPLL_LOG_INFO("Failed to read. Urc=%d", urc);
          msghdr->result_code = urc;
          if (*user_resp_ckv) {
            delete *user_resp_ckv;
            *user_resp_ckv = NULL;
          }
          delete step_req_ckv;
          return urc;
      }
    } else {
      urc = ReadBulkGetSibling(*msghdr, step_req_ckv, &step_resp_ckv,
                               begin, dmi);
    }
    switch (urc) {
      case UPLL_RC_SUCCESS:
        {
          delete step_req_ckv;
          step_req_ckv = NULL;

          ConfigKeyVal *subtree_req_ckv = step_resp_ckv->DupKey();

          if (retrieve_user_req_mo) {
            retrieve_user_req_mo = false;
            delete step_resp_ckv;
            step_resp_ckv = NULL;
          } else {
            // Add the node;
            added_cnt++;
            pending_cnt--;
            if (*user_resp_ckv == NULL) {
              *user_resp_ckv = step_resp_ckv;
            } else {
              (*user_resp_ckv)->AppendCfgKeyVal(step_resp_ckv);
            }
          }

          begin = false;
          ConfigKeyVal *subtree_resp_ckv = NULL;
          uint32_t step_added_cnt = 0;
          upll_rc_t urc = ReadBulkGetSubtree(*kt_tree, *msghdr, cfg_mode,
                                             subtree_req_ckv, pending_cnt,
                                             &subtree_resp_ckv,
                                             &step_added_cnt, dmi);
          if (urc != UPLL_RC_SUCCESS) {
            UPLL_LOG_INFO("ReadBulkGetSubtree failed. Urc=%d", urc);
            delete subtree_req_ckv;
            subtree_req_ckv = NULL;
            if (*user_resp_ckv) {
              delete *user_resp_ckv;
              *user_resp_ckv = NULL;
            }
            msghdr->result_code = urc;
            return urc;
          }
          if (step_added_cnt != 0) {
            added_cnt += step_added_cnt;
            pending_cnt -= step_added_cnt;
            if (*user_resp_ckv == NULL) {
              *user_resp_ckv = subtree_resp_ckv;   // init user_resp_ckv
            } else {
              (*user_resp_ckv)->AppendCfgKeyVal(subtree_resp_ckv);
            }
          }
          if (pending_cnt > 0) {
            // continue; and fetch more
            step_req_ckv = subtree_req_ckv->DupKey();
            delete subtree_req_ckv;
            subtree_req_ckv = NULL;
          } else {
            /* Can't delete step_resp_ckv as is it attached to user_resp_ckv
            delete step_resp_ckv;
            */
            delete subtree_req_ckv;
            subtree_req_ckv = NULL;
            msghdr->result_code = UPLL_RC_SUCCESS;
            msghdr->rep_count = added_cnt;
            UPLL_LOG_TRACE("Returning from %s", __FUNCTION__);
            return UPLL_RC_SUCCESS;
          }
        }
        break;
      case UPLL_RC_ERR_NO_SUCH_INSTANCE:
        {
          // requested key instance is not found
          UPLL_LOG_TRACE("No instance found. GetNextSinlingKT %d",
                         step_req_ckv->get_key_type());
          unc_key_type_t next_sibling_kt;
          if (GetNextSiblingKT(*kt_tree, step_req_ckv->get_key_type(),
                               &next_sibling_kt, msghdr->datatype, cfg_mode)) {
            // There is a NEXT SIBLING KT for the given KT; get the first
            // instance of the next KT
            begin = true;
            ConfigKeyVal *next_req_ckv = GetCkvFromSibling(next_sibling_kt,
                                                           step_req_ckv);
            delete step_req_ckv;
            step_req_ckv = next_req_ckv;
            next_req_ckv = NULL;
            if (step_req_ckv == NULL) {
              // Something wrong?
              UPLL_LOG_INFO("Could not initialize next sibling ckv for %u",
                            next_sibling_kt);
              if (*user_resp_ckv) {
                delete *user_resp_ckv;
                *user_resp_ckv = NULL;
              }
              msghdr->result_code = UPLL_RC_ERR_GENERIC;
              UPLL_LOG_TRACE("Returning from %s", __FUNCTION__);
              return msghdr->result_code;
            }
          } else {
            // There is no NEXT SIBLING KT for the given KT; Get Parent next
            // sibling instance
            UPLL_LOG_TRACE("No next sibling KT found. Get parent for %d",
                           step_req_ckv->get_key_type());
            begin = false;
            unc_key_type_t step_kt = step_req_ckv->get_key_type();
            unc_key_type_t parent_kt;
            bool done = false;
            if (kt_tree->GetParent(step_kt, &parent_kt) == false) {
              UPLL_LOG_INFO("Could not get parent KT for %u", step_kt);
              done = true;
              msghdr->result_code = UPLL_RC_ERR_GENERIC;
            } else if (parent_kt == UNC_KT_ROOT) {
              UPLL_LOG_TRACE("Parent is KT_ROOT for %u", step_kt);
              done = true;
              msghdr->rep_count = added_cnt;
              msghdr->result_code = (added_cnt == 0) ?
                  UPLL_RC_ERR_NO_SUCH_INSTANCE : UPLL_RC_SUCCESS;
            } else {
              ConfigKeyVal *parent_ckv = GetParentCkvFromChild(step_req_ckv);
              if (parent_ckv == NULL) {
                UPLL_LOG_INFO("Could not get parent ckv for %u", step_kt);
                done = true;
                msghdr->result_code = UPLL_RC_ERR_GENERIC;
              } else {
                delete step_req_ckv;
                step_req_ckv = parent_ckv;
                parent_ckv = NULL;
              }
            }
            if (done) {
              delete step_req_ckv;
              step_req_ckv = NULL;

              if ((msghdr->result_code != UPLL_RC_SUCCESS &&
                   msghdr->result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE) &&
                  (*user_resp_ckv != NULL)) {
                delete *user_resp_ckv;
                *user_resp_ckv = NULL;
              }
              return msghdr->result_code;
            }
          }
        }
        continue;
        break;
      default:  // all other error codes
        if (*user_resp_ckv) {
          delete *user_resp_ckv;
          *user_resp_ckv = NULL;
        }
        if (step_req_ckv) {
          delete step_req_ckv;
        }
        msghdr->result_code = urc;
        UPLL_LOG_TRACE("Returning from %s", __FUNCTION__);
        return urc;
    }
  }

  if (*user_resp_ckv) {
    delete *user_resp_ckv;
    *user_resp_ckv = NULL;
  }
  if (step_req_ckv) {
    delete step_req_ckv;
  }
  msghdr->rep_count = 0;
  msghdr->result_code = UPLL_RC_ERR_GENERIC;
  UPLL_LOG_TRACE("Returning from %s", __FUNCTION__);
  return UPLL_RC_ERR_GENERIC;
}
                                                                       // NOLINT
}  // namespace config_momgr
}  // namespace upll
}  // namespace unc
