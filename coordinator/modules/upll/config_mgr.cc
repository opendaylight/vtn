/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * config_mgr.cc - UPLL Config Manager
 */

#include "pfc/debug.h"
#include "pfc/log.h"
#include "unc/component.h"
#include "alarm.hh"
#include "uncxx/lib_unc.hh"

#include "pfcxx/module.hh"

#include "unc/upll_svc.h"
#include "unc/upll_errno.h"
#include "unc/upll_ipc_enum.h"
#include "unc/pfcdriver_include.h"
#include "unc/uppl_common.h"

#include "ipct_st.hh"
#include "uncxx/upll_log.hh"
#include "upll_util.hh"
#include "kt_util.hh"

#include "ctrlr_mgr.hh"
#include "tclib_intf_impl.hh"
#include "config_mgr.hh"

#include "vtn_momgr.hh"
#include "vtn_flowfilter_entry_momgr.hh"
#include "vtn_flowfilter_momgr.hh"
#include "vtn_policingmap_momgr.hh"
#include "vbr_momgr.hh"
#include "vbr_flowfilter_entry_momgr.hh"
#include "vbr_flowfilter_momgr.hh"
#include "vbr_policingmap_momgr.hh"
#include "vbr_if_momgr.hh"
#include "vbr_if_flowfilter_entry_momgr.hh"
#include "vbr_if_flowfilter_momgr.hh"
#include "vbr_if_policingmap_momgr.hh"
#include "vlanmap_momgr.hh"
#include "vrt_momgr.hh"
#include "vrt_if_momgr.hh"
#include "iproute_momgr.hh"
#include "nwm_host_momgr.hh"
#include "nwm_momgr.hh"
#include "dhcprelay_if_momgr.hh"
#include "dhcprelay_server_momgr.hh"
#include "flowlist_entry_momgr.hh"
#include "flowlist_momgr.hh"
#include "policingprofile_entry_momgr.hh"
#include "policingprofile_momgr.hh"
#include "vlink_momgr.hh"
#include "vrt_if_flowfilter_entry_momgr.hh"
#include "vrt_if_flowfilter_momgr.hh"
#include "vunk_momgr.hh"
#include "vunk_if_momgr.hh"
#include "vtep_momgr.hh"
#include "vtep_if_momgr.hh"
#include "vtep_grp_momgr.hh"
#include "vtepgrp_mem_momgr.hh"
#include "vtunnel_momgr.hh"
#include "vtunnel_if_momgr.hh"
#include "vtn_dataflow_momgr.hh"
#include "vterminal_momgr.hh"
#include "vterm_if_momgr.hh"
#include "vterm_if_policingmap_momgr.hh"
#include "vterm_if_flowfilter_entry_momgr.hh"
#include "vterm_if_flowfilter_momgr.hh"
#include "upll_validation.hh"
#include "vbr_portmap_momgr.hh"
#include "unified_nw_momgr.hh"
#include "unw_label_momgr.hh"
#include "unw_label_range_momgr.hh"
#include "vtn_unified_momgr.hh"
#include "unw_spine_domain_momgr.hh"

namespace {
const char * const db_conn_conf_blk = "db_conn";
const uint32_t default_db_max_ro_conns = 64;
const char * const batch_config_mode_conf_blk = "batch_config_mode";
const uint32_t default_batch_timeout = 10;  // in seconds
const uint32_t default_batch_commit_limit = 1000;
const char * const tx_update_taskq_conf_blk = "concurrent_req_to_ctrlrs";
const uint32_t default_tx_update_taskqs = 4;
const char * const oper_status_setting_conf_blk = "oper_status_setting";
const bool default_map_physical_resource_status = true;
}

namespace unc {
namespace upll {
namespace config_momgr {

using unc::unclib::UncModeUtil;
using unc::upll::ipc_util::IpctSt;
using unc::upll::ipc_util::IpcUtil;
using unc::upll::ipc_util::ConfigKeyVal;
using unc::upll::ipc_util::ConfigVal;
using unc::upll::ipc_util::IpcReqRespHeader;
using unc::upll::ipc_util::IpcRequest;
using unc::upll::ipc_util::IpcResponse;
using unc::upll::ipc_util::KtUtil;
using unc::upll::upll_util::upll_strncpy;

namespace uuu = unc::upll::upll_util;
namespace uuk = unc::upll::kt_momgr;
namespace uudal = unc::upll::dal;

UpllConfigMgr *UpllConfigMgr::singleton_instance_;

/*
 * UpllConfigMgr::UpllConfigMgr
 * Constructor of UpllConfigMgr instance.
 */
UpllConfigMgr::UpllConfigMgr()
    : cktt_(UNC_KT_ROOT), iktt_(UNC_KT_ROOT), tclib_impl_(this),
      batch_timeout_fctr_(), batch_timer_func_(batch_timeout_fctr_),
      import_err_behavior_(UNC_IMPORT_ERROR_MODE) {
      UPLL_LOG_DEBUG("constructor");
      commit_in_progress_ = false;
      audit_in_progress_ = false;
      import_in_progress_ = false;
      audit_cancelled_ = false;
      shutting_down_ = false;
      node_active_ = false;
      cl_switched_ = false;
      dbcm_ = NULL;
      alarm_fd = -1;
      audit_ctrlr_affected_state_  = kCtrlrAffectedNoDiff;
      batch_mode_in_progress_ = false;
      batch_timer_ = NULL;
      batch_taskq_ = NULL;
      batch_timeout_id_ = PFC_TIMER_INVALID_ID;
      batch_timeout_ = default_batch_timeout;
      batch_commit_limit_ = default_batch_commit_limit;
      batch_op_cnt_ = 0;
      cur_batch_cfg_mode_ = TC_CONFIG_INVALID;
      cur_batch_cfg_mode_vtn_name_.clear();
      map_phy_resource_status_ = default_map_physical_resource_status;
      import_state_progress = 0;
      current_import_type = UPLL_IMPORT_TYPE_FULL;
      import_type = UPLL_IMPORT_TYPE_FULL;
      tx_util_ = NULL;
      fifo_scheduler_ = NULL;
}

/*
 * UpllConfigMgr::~UpllConfigMgr(void)
 * Destructor of UpllConfigMgr instance.
 */
UpllConfigMgr::~UpllConfigMgr(void) {
  UPLL_LOG_DEBUG("destructor");
  singleton_instance_ = NULL;
  if (alarm_fd != -1) {
    pfc::alarm::pfc_alarm_close(alarm_fd);
  }
  delete tx_util_;
}

/*
 * pfc_bool_t
 * UpllConfigMgr::init(void)
 * Initialize UpllConfigMgr.
 */
bool UpllConfigMgr::Init(void) {
  UPLL_LOG_TRACE("init() called");

  // initialize alarm object
  pfc::alarm::alarm_return_code_t alarm_ret;
  alarm_ret = pfc::alarm::pfc_alarm_initialize(&alarm_fd);
  if (alarm_ret != pfc::alarm::ALM_OK) {
    UPLL_LOG_ERROR("Error %d on alarm initialization", alarm_ret);
    return false;
  }

  // FIFO scheduler enforces sequential execution of priority tasks in UPLL
  fifo_scheduler_ = new TaskScheduler();
  if (!(fifo_scheduler_->Init())) {
    UPLL_LOG_ERROR("Error creating FIFO task scheduler.");
    return false;
  }

  if (!BuildKeyTree())
    return false;

#ifdef PFC_VERBOSE_DEBUG
  DumpKeyTree();
#endif

  unc::upll::ipc_util::IpctSt::RegisterAll();

  dbcm_= new UpllDbConnMgr(GetDbROConnLimitFrmConfFile());

  if (!CreateMoManagers())
    return false;
  if (!InitKtView())
    return false;

  unc::tclib::TcLibModule *tclib = GetTcLibModule();
  PFC_ASSERT(tclib != NULL);
  if (tclib == NULL) {
    UPLL_LOG_ERROR("Unable to get tclib module");
    return false;
  }
  unc::tclib::TcApiCommonRet tacr = tclib->TcLibRegisterHandler(&tclib_impl_);
  if (tacr != unc::tclib::TC_API_COMMON_SUCCESS) {
    UPLL_LOG_ERROR("Failed to register handler with tclib");
    return false;
  }

  GetBatchParamsFrmConfFile();
  GetOperStatusSettingsFromConfFile();
  batch_taskq_= pfc::core::TaskQueue::create(1);
  if (batch_taskq_ == NULL) {
    UPLL_LOG_ERROR("BATCH TaskQ creation failed");
    return false;
  }
  batch_timer_ = pfc::core::Timer::create(batch_taskq_->getId());
  if (batch_timer_ == NULL) {
    UPLL_LOG_ERROR("BATCH Timer creation failed");
    return false;
  }

  tx_util_ = new TxUpdateUtil(GetTxUpdateTaskqParamsFrmConfFile());
  tx_util_->Init();
  pfc_sem_init(&sem_alarm_commit_syc_, 1);

  // Get Import Mode from unclib
  int ret_code = -1;
  UncModeUtil uncModeUtil(ret_code);
  if (0 != ret_code) {
    UPLL_LOG_ERROR("Fail to access unclib.conf default import-mode taken");
  } else {
    import_err_behavior_ = (UncImportMode)uncModeUtil.libunc_get_import_mode();
  }
  UPLL_LOG_DEBUG("Import mode is : %d", import_err_behavior_);

  return true;
}


uint32_t UpllConfigMgr::GetDbROConnLimitFrmConfFile() {
  UPLL_FUNC_TRACE;
  uint32_t max_ro_conn = default_db_max_ro_conns;

  pfc::core::ModuleConfBlock db_conn_block(db_conn_conf_blk);
  if (db_conn_block.getBlock() != PFC_CFBLK_INVALID) {
    UPLL_LOG_TRACE("Block handle is valid");
    max_ro_conn = db_conn_block.getUint32("db_conn_ro_limit",
                                          default_db_max_ro_conns);
    UPLL_LOG_INFO("Max. DB RO connections from conf file %d", max_ro_conn);
  } else {
    UPLL_LOG_INFO("Setting default value to maximum DB RO connections %d",
                  max_ro_conn);
  }
  return max_ro_conn;
}

void UpllConfigMgr::GetOperStatusSettingsFromConfFile() {
  UPLL_FUNC_TRACE;
  pfc::core::ModuleConfBlock oper_status_block(oper_status_setting_conf_blk);
  if (oper_status_block.getBlock() != PFC_CFBLK_INVALID) {
    map_phy_resource_status_ = oper_status_block.getBool(
        "map_physical_resource_status", default_map_physical_resource_status);
    UPLL_LOG_INFO("map_phy_resource_status_ setting from conf file %d",
                   map_phy_resource_status_);
  } else {
    UPLL_LOG_INFO("Setting default value to operstatus processing %d",
                  map_phy_resource_status_);
  }
}

const unc::capa::CapaIntf *UpllConfigMgr::GetCapaInterface() {
  unc::capa::CapaIntf *capa = reinterpret_cast<unc::capa::CapaIntf *>(
      pfc::core::Module::getInstance("capa"));
  if (capa == NULL) {
    UPLL_LOG_FATAL("CapaModule is not found");
  }
  return capa;
}

unc::tclib::TcLibModule *UpllConfigMgr::GetTcLibModule() {
  unc::tclib::TcLibModule *tclib = static_cast<unc::tclib::TcLibModule *>(
      pfc::core::Module::getInstance("tclib"));
  if (tclib == NULL) {
    UPLL_LOG_FATAL("TcLibModule is not found");
  }
  return tclib;
}

upll_rc_t UpllConfigMgr::GetConfigMode(
    uint32_t clnt_sess_id, uint32_t config_id,
    TcConfigMode *cfg_mode, std::string *vtn_name) {
  UPLL_FUNC_TRACE;
  unc::tclib::TcLibModule *tclib = GetTcLibModule();
  if (!tclib) {
    return UPLL_RC_ERR_GENERIC;
  }
  uint32_t tc_err = tclib->TcLibGetConfigMode(clnt_sess_id, config_id,
                                              *cfg_mode, *vtn_name);
  switch (tc_err) {
    case unc::tclib::TC_API_COMMON_SUCCESS:
      return UPLL_RC_SUCCESS;
    case unc::tclib::TC_INVALID_SESSION_ID:
    case unc::tclib::TC_INVALID_CONFIG_ID:
      return UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID;
    default:
      UPLL_LOG_ERROR("Unexpected error %d from TcLibGetConfigMode", tc_err);
      return UPLL_RC_ERR_GENERIC;
  }
}


// IPC service handler
//
pfc_ipcresp_t UpllConfigMgr::KtServiceHandler(upll_service_ids_t service,
                                              IpcReqRespHeader *msghdr,
                                              ConfigKeyVal *first_ckv) {
  UPLL_FUNC_TRACE;
  if (msghdr == NULL || first_ckv == NULL) {
    UPLL_LOG_DEBUG("Null argument %p, %p", msghdr, first_ckv);
    if (msghdr) {
      msghdr->result_code = UPLL_RC_ERR_GENERIC;
    }
    return 0;
  }

  UPLL_LOG_TRACE("In UpllConfigMgr::ServiceHandler "
                 " service=%u\nRequest: %s\n%s",
                 service, IpcUtil::IpcRequestToStr(*msghdr).c_str(),
                 first_ckv->ToStrAll().c_str());

  upll_rc_t urc = ValidateKtRequest(service, *msghdr, *first_ckv);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Validation failed for KT request\n%s\n%s",
                   IpcUtil::IpcRequestToStr(*msghdr).c_str(),
                   first_ckv->ToStrAll().c_str());
    msghdr->result_code = urc;
    return 0;
  }

  // Find MoManager for first_ckv->get_key_type()
  MoManager *momgr = NULL;
  bool momgr_required = true;
  if ((msghdr->operation == UNC_OP_READ_NEXT ||
       msghdr->operation == UNC_OP_READ_BULK)) {
    if (first_ckv->get_key_type() == UNC_KT_ROOT) {
      momgr_required = false;
    }
  }
  if (momgr_required) {
    momgr = GetMoManager(first_ckv->get_key_type());
    if (momgr == NULL) {
      UPLL_LOG_DEBUG("KT %u is not managed by UPLL", first_ckv->get_key_type());
      msghdr->result_code = UPLL_RC_ERR_NO_SUCH_NAME;
      return 0;
    }
  }

  if ((msghdr->operation == UNC_OP_READ_BULK ||
       msghdr->operation == UNC_OP_READ_SIBLING_BEGIN ||
       msghdr->operation == UNC_OP_READ_SIBLING)) {
    if (msghdr->rep_count == 0) {
      UPLL_LOG_TRACE("MaxRepCount is zero. Nothing to return");
      msghdr->result_code = UPLL_RC_SUCCESS;
      return 0;
    } else if (msghdr->rep_count > MoManager::kMaxReadBulkCount) {
      UPLL_LOG_TRACE("MaxRepCount is %u. Reducing it to %u", msghdr->rep_count,
                     MoManager::kMaxReadBulkCount);
      msghdr->rep_count = MoManager::kMaxReadBulkCount;
    }
  }

  // Create/update/delete are crit. tasks so add a task in prior_taskq_
  // of FIFO scheduler. ConfigLock should be taken after scheduling the
  // task to avoid deadlocks between tasks in prior_taskq.
  TaskScheduler::Task *task = NULL;
  if (msghdr->operation == UNC_OP_CREATE ||
      msghdr->operation == UNC_OP_UPDATE ||
      msghdr->operation == UNC_OP_DELETE) {
    task = fifo_scheduler_->AllowExecution(kCriticalTaskPriority, __FUNCTION__);
    if (!task) {
      UPLL_LOG_FATAL("Failed to schedule priority operation -"
                     " service=%u\nRequest: %s\n%s",
                     service, IpcUtil::IpcRequestToStr(*msghdr).c_str(),
                     first_ckv->ToStrAll().c_str());
      return UPLL_RC_ERR_GENERIC;
    }
  }

  // Rename should not be allowed when start_import or merge or
  // clear_import is in progress. import_mutex_lock_ should be
  // taken before taking lock on DT_IMPORT
  if (msghdr->operation == UNC_OP_RENAME)
    pfc::core::ScopedMutex lock(import_mutex_lock_);

  // Take configuration locks
  if (TakeConfigLock(msghdr->operation, msghdr->datatype) == false) {
    msghdr->result_code = UPLL_RC_ERR_GENERIC;
    UPLL_LOG_DEBUG("Failed to take config lock.");
    return 0;
  }

  // Open Dal Connection
  DalOdbcMgr *dom = NULL;
  bool dom_commit_flag = false;
  switch (msghdr->operation) {
    case UNC_OP_CREATE:
    case UNC_OP_UPDATE:
    case UNC_OP_DELETE:
    case UNC_OP_RENAME:
      dom = dbcm_->GetConfigRwConn();
      if (dom == NULL) {
        UPLL_LOG_ERROR("Failed to get dom");
        msghdr->result_code = UPLL_RC_ERR_GENERIC;
        if (ReleaseConfigLock(msghdr->operation, msghdr->datatype) == false) {
          UPLL_LOG_ERROR("Failed to release config lock.");
        }
        return 0;
      }
      dom->reset_write_count();
      dom_commit_flag = true;
      break;
    case UNC_OP_READ:
    case UNC_OP_READ_NEXT:
    case UNC_OP_READ_BULK:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_COUNT:
      if (msghdr->datatype == UPLL_DT_CANDIDATE) {
        dom = dbcm_->GetConfigRwConn();
        if (dom == NULL) {
          UPLL_LOG_ERROR("Failed to get dom");
          msghdr->result_code = UPLL_RC_ERR_GENERIC;
          if (ReleaseConfigLock(msghdr->operation, msghdr->datatype) == false) {
            UPLL_LOG_ERROR("Failed to release config lock.");
          }
          return 0;
        }
      }
      break;
    default:
      // UNC_OP_CONTROL and other UNC_OP_READ* operations
      // For now use read only connection.
      dom = NULL;
  }

  if (dom != NULL) {
    // already selected the dom
  } else {
    if (UPLL_RC_SUCCESS != (urc = dbcm_->AcquireRoConn(&dom))) {
      msghdr->result_code = urc;
      if (ReleaseConfigLock(msghdr->operation, msghdr->datatype) == false) {
        UPLL_LOG_ERROR("Failed to release config lock.");
      }
      return 0;
    }
  }

  switch (msghdr->operation) {
    case UNC_OP_READ:
      urc = momgr->ReadMo(msghdr, first_ckv, dom);
      break;
    case UNC_OP_READ_NEXT:
      urc = ReadNextMo(msghdr, first_ckv, dom);
      break;
    case UNC_OP_READ_BULK:
      urc = ReadBulkMo(msghdr, first_ckv, dom);
      break;
    case UNC_OP_READ_SIBLING_BEGIN:
      urc = momgr->ReadSiblingMo(msghdr, first_ckv, true, dom);
      break;
    case UNC_OP_READ_SIBLING:
      urc = momgr->ReadSiblingMo(msghdr, first_ckv, false, dom);
      break;
    case UNC_OP_READ_SIBLING_COUNT:
      {
        urc = momgr->ReadSiblingCount(msghdr, first_ckv, dom);
        if (urc == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
          urc = msghdr->result_code = UPLL_RC_SUCCESS;
          uint32_t *count = reinterpret_cast<uint32_t *>(
              ConfigKeyVal::Malloc(sizeof(uint32_t)));
          *count = 0;
          first_ckv->SetCfgVal(new ConfigVal(IpctSt::kIpcStUint32, count));
        }
      }
      break;
    case UNC_OP_CREATE:
      urc = momgr->CreateMo(msghdr, first_ckv, dom);
      break;
    case UNC_OP_UPDATE:
      urc = momgr->UpdateMo(msghdr, first_ckv, dom);
      break;
    case UNC_OP_DELETE:
      urc = momgr->DeleteMo(msghdr, first_ckv, dom);
      break;
    case UNC_OP_RENAME:
      if (import_ctrlr_id_.empty()) {
        urc = UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
        UPLL_LOG_DEBUG("Reanme operation is not allowed as "
                       "import operation not in progress");
      } else {
        urc = momgr->RenameMo(msghdr, first_ckv, dom,
                              import_ctrlr_id_.c_str());
      }
      break;
    case UNC_OP_CONTROL:
      urc = momgr->ControlMo(msghdr, first_ckv, dom);
      break;
    default:
      UPLL_LOG_DEBUG("Unknown keytype operation: %d", msghdr->operation);
      urc = UPLL_RC_ERR_NO_SUCH_OPERATION;
  }

  // Completion of critical priority task
  if (task != NULL) {
    if (!(fifo_scheduler_->DoneExecution(task))) {
      UPLL_LOG_FATAL("Failed to complete the priority task");
      return UPLL_RC_ERR_GENERIC;
    }
  }

  msghdr->result_code = urc;

  if (dom->get_conn_type() == uudal::kDalConnReadWrite) {
    upll_rc_t db_urc = UPLL_RC_SUCCESS;
    // NOTE: XXX: Take only batch_mutex_lock here and do not take any other
    // config locks in any other functions called from here.
    batch_mutex_lock_.lock();
    if (!batch_mode_in_progress_) {
      db_urc = dbcm_->DalTxClose(dom, ((urc == UPLL_RC_SUCCESS) &&
                                       (dom_commit_flag == true)));
    } else {
      switch (msghdr->operation) {
        case UNC_OP_CREATE:
        case UNC_OP_UPDATE:
        case UNC_OP_DELETE:
        case UNC_OP_RENAME:
          if ((urc != UPLL_RC_SUCCESS) && (dom->get_write_count() > 0)) {
            // do rollback - entire batch config done so far is rolled back.
            db_urc = dbcm_->DalTxClose(dom, false);
          } else {
            // Do DB commit if batch_commit_limit_ is reached
            batch_op_cnt_++;
            if (batch_op_cnt_ >= batch_commit_limit_) {
              db_urc = dbcm_->DalTxClose(dom, true);
              batch_op_cnt_ = 0;
            }
          }
          break;
        default:
          // READ* on DT_CANDIDATE versus rest of the operations
          if (msghdr->datatype == UPLL_DT_CANDIDATE) {
            // do nothing - cannot commit while in batch mode.
            // NOTE: XXX: On READ failure, no need to rollback.
          } else {
            // NOTE: Depending on RW connection usage, this might be dead code,
            // but needed
            db_urc = dbcm_->DalTxClose(dom, ((urc == UPLL_RC_SUCCESS) &&
                                             (dom_commit_flag == true)));
          }
      }
    }
    dbcm_->ReleaseRwConn(dom);
    if (urc == UPLL_RC_SUCCESS) {
      msghdr->result_code = urc = db_urc;
    }
    batch_mutex_lock_.unlock();
  } else {
    // RO Connection processing - Rollback and release the connection
    dbcm_->DalTxClose(dom, false);
    dbcm_->ReleaseRoConn(dom);
  }

  // Release configuration locks
  if (ReleaseConfigLock(msghdr->operation, msghdr->datatype) == false) {
    UPLL_LOG_ERROR("Failed to release config lock.");
  }

  if (msghdr->result_code != UPLL_RC_SUCCESS &&
      msghdr->result_code != UPLL_RC_ERR_INSTANCE_EXISTS &&
      msghdr->result_code != UPLL_RC_ERR_NO_SUCH_INSTANCE &&
      msghdr->result_code != UPLL_RC_ERR_CTR_DISCONNECTED &&
      msghdr->result_code != UPLL_RC_ERR_DRIVER_NOT_PRESENT) {
    UPLL_LOG_INFO("In UpllConfigMgr::ServiceHandler "
                  " service=%u\nResponse: %s\n%s",
                  service, IpcUtil::IpcResponseToStr(*msghdr).c_str(),
                  first_ckv->ToStrAll().c_str());
  } else {
    UPLL_LOG_DEBUG("In UpllConfigMgr::ServiceHandler "
                   " service=%u\nResponse: %s\n%s",
                   service, IpcUtil::IpcResponseToStr(*msghdr).c_str(),
                   first_ckv->ToStrAll().c_str());
  }

  return 0;
}

bool UpllConfigMgr::BuildKeyTree() {
  UPLL_FUNC_TRACE;
  // Init Regualr Config KeyType Tree
    if (cktt_.AddKeyType(UNC_KT_ROOT, UNC_KT_UNIFIED_NETWORK) &&
      (
          cktt_.AddKeyType(UNC_KT_UNIFIED_NETWORK, UNC_KT_UNW_LABEL) &&
          cktt_.AddKeyType(UNC_KT_UNW_LABEL, UNC_KT_UNW_LABEL_RANGE) &&
          cktt_.AddKeyType(UNC_KT_UNIFIED_NETWORK, UNC_KT_UNW_SPINE_DOMAIN)) &&
      cktt_.AddKeyType(UNC_KT_ROOT, UNC_KT_FLOWLIST) &&
      (
          cktt_.AddKeyType(UNC_KT_FLOWLIST, UNC_KT_FLOWLIST_ENTRY)
      ) &&                                                             // NOLINT
      cktt_.AddKeyType(UNC_KT_ROOT, UNC_KT_POLICING_PROFILE) &&
      (
          cktt_.AddKeyType(UNC_KT_POLICING_PROFILE,
                           UNC_KT_POLICING_PROFILE_ENTRY)
      ) &&                                                             // NOLINT
      // skip UNC_KT_PATHMAP_ENTRY
      // skip UNC_KT_PATHMAP_CONTROLLER
      // skip UNC_KT_VTNSTATION_CONTROLLER
      cktt_.AddKeyType(UNC_KT_ROOT, UNC_KT_VTN) &&
      (
          cktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VTN_UNIFIED) &&
          // skip UNC_KT_VTN_DATAFLOW_CONTROLLER
          cktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VBRIDGE) &&
          (
              cktt_.AddKeyType(UNC_KT_VBRIDGE, UNC_KT_VBR_PORTMAP) &&
              cktt_.AddKeyType(UNC_KT_VBRIDGE, UNC_KT_VBR_VLANMAP) &&
              cktt_.AddKeyType(UNC_KT_VBRIDGE, UNC_KT_VBR_NWMONITOR) &&
              cktt_.AddKeyType(UNC_KT_VBR_NWMONITOR,
                               UNC_KT_VBR_NWMONITOR_HOST) &&
              cktt_.AddKeyType(UNC_KT_VBRIDGE, UNC_KT_VBR_POLICINGMAP) &&
              // skip UNC_KT_VBR_POLICINGMAP_ENTRY
              cktt_.AddKeyType(UNC_KT_VBRIDGE, UNC_KT_VBR_FLOWFILTER) &&
              cktt_.AddKeyType(UNC_KT_VBR_FLOWFILTER,
                               UNC_KT_VBR_FLOWFILTER_ENTRY) &&
              cktt_.AddKeyType(UNC_KT_VBRIDGE, UNC_KT_VBR_IF) &&
              // skip UNC_KT_IF_MACENTRY
              cktt_.AddKeyType(UNC_KT_VBR_IF, UNC_KT_VBRIF_POLICINGMAP) &&
              // skip UNC_KT_VBRIF_POLICINGMAP_ENTRY
              cktt_.AddKeyType(UNC_KT_VBR_IF, UNC_KT_VBRIF_FLOWFILTER) &&
              cktt_.AddKeyType(UNC_KT_VBRIF_FLOWFILTER,
                               UNC_KT_VBRIF_FLOWFILTER_ENTRY)
          ) &&                                                         // NOLINT
          cktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VROUTER) &&
          (
              cktt_.AddKeyType(UNC_KT_VROUTER, UNC_KT_VRT_IF) &&
              cktt_.AddKeyType(UNC_KT_VRT_IF, UNC_KT_VRTIF_FLOWFILTER) &&
              cktt_.AddKeyType(UNC_KT_VRTIF_FLOWFILTER,
                               UNC_KT_VRTIF_FLOWFILTER_ENTRY) &&
              cktt_.AddKeyType(UNC_KT_VROUTER, UNC_KT_VRT_IPROUTE) &&
              cktt_.AddKeyType(UNC_KT_VROUTER, UNC_KT_DHCPRELAY_SERVER) &&
              cktt_.AddKeyType(UNC_KT_VROUTER, UNC_KT_DHCPRELAY_IF)
              // skip UNC_KT_IF_ARPENTRY
          ) &&                                                         // NOLINT
          cktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VTERMINAL) &&
          (
              cktt_.AddKeyType(UNC_KT_VTERMINAL, UNC_KT_VTERM_IF) &&
              cktt_.AddKeyType(UNC_KT_VTERM_IF, UNC_KT_VTERMIF_POLICINGMAP) &&
              // skip UNC_KT_VTERMIF_POLICINGMAP_ENTRY
              cktt_.AddKeyType(UNC_KT_VTERM_IF, UNC_KT_VTERMIF_FLOWFILTER) &&
              cktt_.AddKeyType(UNC_KT_VTERMIF_FLOWFILTER,
                               UNC_KT_VTERMIF_FLOWFILTER_ENTRY)
          ) &&                                                         // NOLINT
          cktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VUNKNOWN) &&
          cktt_.AddKeyType(UNC_KT_VUNKNOWN, UNC_KT_VUNK_IF) &&
          cktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VTEP) &&
          cktt_.AddKeyType(UNC_KT_VTEP, UNC_KT_VTEP_IF) &&
          cktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VTEP_GRP) &&
          cktt_.AddKeyType(UNC_KT_VTEP_GRP, UNC_KT_VTEP_GRP_MEMBER) &&
          cktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VTUNNEL) &&
          cktt_.AddKeyType(UNC_KT_VTUNNEL, UNC_KT_VTUNNEL_IF) &&
          cktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VLINK) &&
          cktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VTN_POLICINGMAP) &&
          // skip UNC_KT_VTN_POLICINGMAP_CONTROLLER
          cktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VTN_FLOWFILTER) &&
          (
              cktt_.AddKeyType(UNC_KT_VTN_FLOWFILTER,
                               UNC_KT_VTN_FLOWFILTER_ENTRY)
          )
          // skip UNC_KT_VTN_FLOWFILTER_CONTROLLER
          // skip UNC_KT_VTN_PATHMAP_ENTRY
          // skip UNC_KT_VTN_PATHMAP_CONTROLLER
          )
          ) {
          } else {
            return false;
          }

  cktt_.PrepareOrderedList();

  // Init Import KeyType Tree
  if (iktt_.AddKeyType(UNC_KT_ROOT, UNC_KT_FLOWLIST) &&
      iktt_.AddKeyType(UNC_KT_ROOT, UNC_KT_POLICING_PROFILE) &&
      iktt_.AddKeyType(UNC_KT_ROOT, UNC_KT_VTN) &&
      (
          iktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VBRIDGE) &&
          iktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VROUTER) &&
          iktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VTERMINAL) &&
          iktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VLINK)
      )
     ) {
  } else {
    return false;
  }

  iktt_.PrepareOrderedList();

  kt_name_map_[UNC_KT_ROOT] = "ROOT";
  kt_name_map_[UNC_KT_UNIFIED_NETWORK] = "UNIFIED_NETWORK";
  kt_name_map_[UNC_KT_UNW_LABEL] = "UNW_LABEL";
  kt_name_map_[UNC_KT_UNW_LABEL_RANGE] = "UNW_LABEL_RANGE";
  kt_name_map_[UNC_KT_UNW_SPINE_DOMAIN] = "UNW_SPINE_DOMAIN";
  kt_name_map_[UNC_KT_FLOWLIST] = "FLOWLIST";
  kt_name_map_[UNC_KT_FLOWLIST_ENTRY] = "FLOWLIST_ENTRY";
  kt_name_map_[UNC_KT_POLICING_PROFILE] = "POLICING_PROFILE";
  kt_name_map_[UNC_KT_POLICING_PROFILE_ENTRY] = "POLICING_PROFILE_ENTRY";
  kt_name_map_[UNC_KT_PATHMAP_ENTRY] = "PATHMAP_ENTRY";
  kt_name_map_[UNC_KT_PATHMAP_CONTROLLER] = "PATHMAP_CONTROLLER";
  kt_name_map_[UNC_KT_VTNSTATION_CONTROLLER] = "VTNSTATION_CONTROLLER";
  kt_name_map_[UNC_KT_VTN] = "VTN";
  kt_name_map_[UNC_KT_VTN_UNIFIED] = "VTN_UNIFIED";
  kt_name_map_[UNC_KT_VTN_MAPPING_CONTROLLER] = "VTN_MAPPING_CONTROLLER";
  kt_name_map_[UNC_KT_VTN_DATAFLOW_CONTROLLER] = "VTN_DATAFLOW_CONTROLLER";
  kt_name_map_[UNC_KT_VTN_FLOWFILTER] = "VTN_FLOWFILTER";
  kt_name_map_[UNC_KT_VTN_FLOWFILTER_ENTRY] = "VTN_FLOWFILTER_ENTRY";
  kt_name_map_[UNC_KT_VTN_FLOWFILTER_CONTROLLER] = "VTN_FLOWFILTER_CONTROLLER";
  kt_name_map_[UNC_KT_VTN_PATHMAP_ENTRY] = "VTN_PATHMAP_ENTRY";
  kt_name_map_[UNC_KT_VTN_PATHMAP_CONTROLLER] = "VTN_PATHMAP_CONTROLLER";
  kt_name_map_[UNC_KT_VTN_POLICINGMAP] = "VTN_POLICINGMAP";
  kt_name_map_[UNC_KT_VTN_POLICINGMAP_CONTROLLER] =
      "VTN_POLICINGMAP_CONTROLLER";
  kt_name_map_[UNC_KT_VBRIDGE] = "VBRIDGE";
  kt_name_map_[UNC_KT_VBR_PORTMAP] = "VBR_PORTMAP";
  kt_name_map_[UNC_KT_VBR_VLANMAP] = "VBR_VLANMAP";
  kt_name_map_[UNC_KT_VBR_NWMONITOR] = "VBR_NWMONITOR";
  kt_name_map_[UNC_KT_VBR_NWMONITOR_HOST] = "VBR_NWMONITOR_HOST";
  kt_name_map_[UNC_KT_VBR_POLICINGMAP] = "VBR_POLICINGMAP";
  kt_name_map_[UNC_KT_VBR_POLICINGMAP_ENTRY] = "VBR_POLICINGMAP_ENTRY";
  kt_name_map_[UNC_KT_VBR_FLOWFILTER] = "VBR_FLOWFILTER";
  kt_name_map_[UNC_KT_VBR_FLOWFILTER_ENTRY] = "VBR_FLOWFILTER_ENTRY";
  kt_name_map_[UNC_KT_VBR_IF] = "VBR_IF";
  kt_name_map_[UNC_KT_IF_MACENTRY] = "IF_MACENTRY";
  kt_name_map_[UNC_KT_VBRIF_FLOWFILTER] = "VBRIF_FLOWFILTER";
  kt_name_map_[UNC_KT_VBRIF_FLOWFILTER_ENTRY] = "VBRIF_FLOWFILTER_ENTRY";
  kt_name_map_[UNC_KT_VBRIF_POLICINGMAP] = "VBRIF_POLICINGMAP";
  kt_name_map_[UNC_KT_VBRIF_POLICINGMAP_ENTRY] = "VBRIF_POLICINGMAP_ENTRY";
  kt_name_map_[UNC_KT_VROUTER] = "VROUTER";
  kt_name_map_[UNC_KT_VRT_IF] = "VRT_IF";
  kt_name_map_[UNC_KT_VRT_IPROUTE] = "IPROUTE";
  kt_name_map_[UNC_KT_DHCPRELAY_SERVER] = "DHCPRELAY_SERVER";
  kt_name_map_[UNC_KT_DHCPRELAY_IF] = "DHCPRELAY_IF";
  kt_name_map_[UNC_KT_IF_ARPENTRY] = "IF_ARPENTRY";
  kt_name_map_[UNC_KT_VRTIF_FLOWFILTER] = "VRTIF_FLOWFILTER";
  kt_name_map_[UNC_KT_VRTIF_FLOWFILTER_ENTRY] = "VRTIF_FLOWFILTER_ENTRY";
  kt_name_map_[UNC_KT_VTERMINAL] = "VTERMINAL";
  kt_name_map_[UNC_KT_VTERM_IF] = "VTERM_IF";
  kt_name_map_[UNC_KT_VTERMIF_FLOWFILTER] = "VTERMIF_FLOWFILTER";
  kt_name_map_[UNC_KT_VTERMIF_FLOWFILTER_ENTRY] = "VTERMIF_FLOWFILTER_ENTRY";
  kt_name_map_[UNC_KT_VTERMIF_POLICINGMAP] = "VTERMIF_POLICINGMAP";
  kt_name_map_[UNC_KT_VTERMIF_POLICINGMAP_ENTRY] = "VTERMIF_POLICINGMAP_ENTRY";
  kt_name_map_[UNC_KT_VUNKNOWN] = "VUNKNOWN";
  kt_name_map_[UNC_KT_VUNK_IF] = "VUNK_IF";
  kt_name_map_[UNC_KT_VTEP] = "VTEP";
  kt_name_map_[UNC_KT_VTEP_IF] = "VTEP_IF";
  kt_name_map_[UNC_KT_VTEP_GRP] = "VTEP_GRP";
  kt_name_map_[UNC_KT_VTEP_GRP_MEMBER] = "VTEP_GRP_MEMBER";
  kt_name_map_[UNC_KT_VTUNNEL] = "VTUNNEL";
  kt_name_map_[UNC_KT_VTUNNEL_IF] = "VTUNNEL_IF";
  kt_name_map_[UNC_KT_VLINK] = "VLINK";
  kt_name_map_[UNC_KT_VTN_DATAFLOW] = "VTN_DATAFLOW";

  return true;
}

void UpllConfigMgr::DumpKeyTree() {
  unc_key_type_t kt;

  UPLL_LOG_TRACE("--- KeyTree Preorder Traversal ---");
  const std::list<unc_key_type_t> *pre_list = cktt_.get_preorder_list();
  std::list<unc_key_type_t>::const_iterator pre_it = pre_list->begin();
  std::map<unc_key_type_t, std::string>::iterator nmap_it;

  while (pre_it != pre_list->end()) {
    kt = *pre_it;
    nmap_it = kt_name_map_.find(kt);
    if (nmap_it !=  kt_name_map_.end()) {
      UPLL_LOG_TRACE("--- KT: %s", nmap_it->second.c_str());
    }
    pre_it++;
  }
  UPLL_LOG_TRACE("--- KeyTree Postorder Traversal ---");
  const std::list<unc_key_type_t> *post_list =
      cktt_.get_reverse_postorder_list();
  std::list<unc_key_type_t>::const_iterator post_it = post_list->begin();
  while (post_it != post_list->end()) {
    kt = *post_it;
    nmap_it = kt_name_map_.find(kt);
    if (nmap_it !=  kt_name_map_.end()) {
      UPLL_LOG_TRACE("--- KT: %s", nmap_it->second.c_str());
    }
    post_it++;
  }
}

bool UpllConfigMgr::CreateMoManagers() {
  UPLL_FUNC_TRACE;

  namespace uuk = unc::upll::kt_momgr;
  upll_kt_momgrs_[UNC_KT_UNIFIED_NETWORK] = new uuk::UnifiedNwMoMgr();
  upll_kt_momgrs_[UNC_KT_UNW_LABEL] = new uuk::UNWLabelMoMgr();
  upll_kt_momgrs_[UNC_KT_UNW_LABEL_RANGE] = new uuk::UNWLabelRangeMoMgr();
  upll_kt_momgrs_[UNC_KT_UNW_SPINE_DOMAIN] = new uuk::UNWSpineDomainMoMgr();
  upll_kt_momgrs_[UNC_KT_VTN] =
  upll_kt_momgrs_[UNC_KT_VTNSTATION_CONTROLLER] =
  upll_kt_momgrs_[UNC_KT_VTN_MAPPING_CONTROLLER] = new uuk::VtnMoMgr();
  upll_kt_momgrs_[UNC_KT_VTN_UNIFIED] = new uuk::VtnUnifiedMoMgr();
  upll_kt_momgrs_[UNC_KT_VBRIDGE] = new uuk::VbrMoMgr();
  upll_kt_momgrs_[UNC_KT_VBR_PORTMAP] = new uuk::VbrPortMapMoMgr();
  upll_kt_momgrs_[UNC_KT_VBR_VLANMAP] = new uuk::VlanMapMoMgr();
  upll_kt_momgrs_[UNC_KT_VBR_NWMONITOR] = new uuk::NwMonitorMoMgr();
  upll_kt_momgrs_[UNC_KT_VBR_NWMONITOR_HOST] = new uuk::NwMonitorHostMoMgr();
  upll_kt_momgrs_[UNC_KT_VBR_IF] = new uuk::VbrIfMoMgr();
  upll_kt_momgrs_[UNC_KT_VROUTER] = new uuk::VrtMoMgr();
  upll_kt_momgrs_[UNC_KT_VRT_IPROUTE] = new uuk::IpRouteMoMgr();
  upll_kt_momgrs_[UNC_KT_DHCPRELAY_SERVER] = new uuk::DhcpRelayServerMoMgr();
  upll_kt_momgrs_[UNC_KT_DHCPRELAY_IF] = new uuk::DhcpRelayIfMoMgr();
  upll_kt_momgrs_[UNC_KT_VRT_IF] = new uuk::VrtIfMoMgr();
  upll_kt_momgrs_[UNC_KT_VTERMINAL] = new uuk::VterminalMoMgr();
  upll_kt_momgrs_[UNC_KT_VTERM_IF] = new uuk::VtermIfMoMgr();
  upll_kt_momgrs_[UNC_KT_VUNKNOWN] = new uuk::VunknownMoMgr();
  upll_kt_momgrs_[UNC_KT_VUNK_IF] = new uuk::VunkIfMoMgr();
  upll_kt_momgrs_[UNC_KT_VTEP] = new uuk::VtepMoMgr();
  upll_kt_momgrs_[UNC_KT_VTEP_IF] = new uuk::VtepIfMoMgr();
  upll_kt_momgrs_[UNC_KT_VTEP_GRP] = new uuk::VtepGrpMoMgr();
  upll_kt_momgrs_[UNC_KT_VTEP_GRP_MEMBER] = new uuk::VtepGrpMemMoMgr();
  upll_kt_momgrs_[UNC_KT_VTUNNEL] = new uuk::VtunnelMoMgr();
  upll_kt_momgrs_[UNC_KT_VTUNNEL_IF] = new uuk::VtunnelIfMoMgr();
  upll_kt_momgrs_[UNC_KT_VLINK] = new uuk::VlinkMoMgr();

  upll_kt_momgrs_[UNC_KT_FLOWLIST] = new uuk::FlowListMoMgr();
  upll_kt_momgrs_[UNC_KT_FLOWLIST_ENTRY] = new uuk::FlowListEntryMoMgr();
  upll_kt_momgrs_[UNC_KT_POLICING_PROFILE] = new uuk::PolicingProfileMoMgr();
  upll_kt_momgrs_[UNC_KT_POLICING_PROFILE_ENTRY] =
      new uuk::PolicingProfileEntryMoMgr();
  upll_kt_momgrs_[UNC_KT_VTN_FLOWFILTER] = new uuk::VtnFlowFilterMoMgr();
  upll_kt_momgrs_[UNC_KT_VTN_FLOWFILTER_ENTRY] =
  upll_kt_momgrs_[UNC_KT_VTN_FLOWFILTER_CONTROLLER] =
      new uuk::VtnFlowFilterEntryMoMgr();
  upll_kt_momgrs_[UNC_KT_VTN_POLICINGMAP] =
  upll_kt_momgrs_[UNC_KT_VTN_POLICINGMAP_CONTROLLER] =
      new uuk::VtnPolicingMapMoMgr();
  upll_kt_momgrs_[UNC_KT_VBR_POLICINGMAP_ENTRY] =
  upll_kt_momgrs_[UNC_KT_VBR_POLICINGMAP] = new uuk::VbrPolicingMapMoMgr();
  upll_kt_momgrs_[UNC_KT_VBR_FLOWFILTER] = new uuk::VbrFlowFilterMoMgr();
  upll_kt_momgrs_[UNC_KT_VBR_FLOWFILTER_ENTRY] =
      new uuk::VbrFlowFilterEntryMoMgr();
  upll_kt_momgrs_[UNC_KT_VBRIF_FLOWFILTER] = new uuk::VbrIfFlowFilterMoMgr();
  upll_kt_momgrs_[UNC_KT_VBRIF_FLOWFILTER_ENTRY] =
      new uuk::VbrIfFlowFilterEntryMoMgr();
  upll_kt_momgrs_[UNC_KT_VBRIF_POLICINGMAP_ENTRY] =
      upll_kt_momgrs_[UNC_KT_VBRIF_POLICINGMAP] =
      new uuk::VbrIfPolicingMapMoMgr();
  upll_kt_momgrs_[UNC_KT_VRTIF_FLOWFILTER] = new uuk::VrtIfFlowFilterMoMgr();
  upll_kt_momgrs_[UNC_KT_VRTIF_FLOWFILTER_ENTRY] =
      new uuk::VrtIfFlowFilterEntryMoMgr();
  upll_kt_momgrs_[UNC_KT_VTERMIF_FLOWFILTER] =
      new uuk::VtermIfFlowFilterMoMgr();
  upll_kt_momgrs_[UNC_KT_VTERMIF_FLOWFILTER_ENTRY] =
      new uuk::VtermIfFlowFilterEntryMoMgr();
  upll_kt_momgrs_[UNC_KT_VTN_DATAFLOW] = new uuk::VtnDataflowMoMgr();
  upll_kt_momgrs_[UNC_KT_VTERMIF_POLICINGMAP_ENTRY] =
  upll_kt_momgrs_[UNC_KT_VTERMIF_POLICINGMAP] =
      new uuk::VtermIfPolicingMapMoMgr();

  return true;
}

bool UpllConfigMgr::InitKtView() {
  /*
  //TODO(U17) ?????????????? KT_ROOT belongs to which view?
  */
  kt_view_[UNC_KT_ROOT] = (UpllKtView) 0xFFFFFFFF;

  for (std::map<unc_key_type_t, MoManager*>::iterator kt_iter
       = upll_kt_momgrs_.begin();
       kt_iter != upll_kt_momgrs_.end(); kt_iter++) {
    unc_key_type_t kt = kt_iter->first;
    switch (kt) {
      case UNC_KT_VTN:
      case UNC_KT_VBRIDGE:
      case UNC_KT_VBR_PORTMAP:
      case UNC_KT_VBR_VLANMAP:
      case UNC_KT_VBR_NWMONITOR:
      case UNC_KT_VBR_NWMONITOR_HOST:
      case UNC_KT_VBR_IF:
      case UNC_KT_VROUTER:
      case UNC_KT_VRT_IPROUTE:
      case UNC_KT_DHCPRELAY_SERVER:
      case UNC_KT_DHCPRELAY_IF:
      case UNC_KT_VRT_IF:
      case UNC_KT_VTERMINAL:
      case UNC_KT_VTERM_IF:
      case UNC_KT_VUNKNOWN:
      case UNC_KT_VUNK_IF:
      case UNC_KT_VTEP:
      case UNC_KT_VTEP_IF:
      case UNC_KT_VTEP_GRP:
      case UNC_KT_VTEP_GRP_MEMBER:
      case UNC_KT_VTUNNEL:
      case UNC_KT_VTUNNEL_IF:
      case UNC_KT_VLINK:
      case UNC_KT_VTN_FLOWFILTER:
      case UNC_KT_VTN_FLOWFILTER_ENTRY:
      case UNC_KT_VTN_FLOWFILTER_CONTROLLER:
      case UNC_KT_VTN_POLICINGMAP:
      case UNC_KT_VTN_POLICINGMAP_CONTROLLER:
      case UNC_KT_VBR_POLICINGMAP_ENTRY:
      case UNC_KT_VBR_POLICINGMAP:
      case UNC_KT_VBR_FLOWFILTER:
      case UNC_KT_VBR_FLOWFILTER_ENTRY:
      case UNC_KT_VBRIF_FLOWFILTER:
      case UNC_KT_VBRIF_FLOWFILTER_ENTRY:
      case UNC_KT_VBRIF_POLICINGMAP_ENTRY:
      case UNC_KT_VBRIF_POLICINGMAP:
      case UNC_KT_VRTIF_FLOWFILTER:
      case UNC_KT_VRTIF_FLOWFILTER_ENTRY:
      case UNC_KT_VTERMIF_FLOWFILTER:
      case UNC_KT_VTERMIF_FLOWFILTER_ENTRY:
      case UNC_KT_VTERMIF_POLICINGMAP_ENTRY:
      case UNC_KT_VTERMIF_POLICINGMAP:
      case UNC_KT_VTN_UNIFIED:
        kt_view_[kt] = kUpllKtViewVtn;
        break;
      case UNC_KT_FLOWLIST:
      case UNC_KT_FLOWLIST_ENTRY:
      case UNC_KT_POLICING_PROFILE:
      case UNC_KT_POLICING_PROFILE_ENTRY:
      case UNC_KT_UNIFIED_NETWORK:
      case UNC_KT_UNW_LABEL:
      case UNC_KT_UNW_LABEL_RANGE:
      case UNC_KT_UNW_SPINE_DOMAIN:
        kt_view_[kt] = kUpllKtViewVirtual;
        break;
      case UNC_KT_VTNSTATION_CONTROLLER:
      case UNC_KT_VTN_MAPPING_CONTROLLER:
      case UNC_KT_VTN_DATAFLOW:
        kt_view_[kt] = kUpllKtViewState;
        break;
      default:
        UPLL_LOG_ERROR("Uknown KT: %d", kt);
        return false;
    }
  }
  // prepare view lists
  const std::list<unc_key_type_t> *lst = cktt_.get_preorder_list();
  for (std::list<unc_key_type_t>::const_iterator it = lst->begin();
       it != lst->end(); it++) {
    const unc_key_type_t kt(*it);
    if (IsKtPartOfConfigMode(kt, UPLL_DT_CANDIDATE, TC_CONFIG_VIRTUAL)) {
      preorder_virtual_kt_list_.push_back(kt);
    }
    if (IsKtPartOfConfigMode(kt, UPLL_DT_CANDIDATE, TC_CONFIG_VTN)) {
      preorder_vtn_kt_list_.push_back(kt);
    }
  }

  return true;
}

bool UpllConfigMgr::IsKtPartOfConfigMode(unc_key_type_t kt,
                                         upll_keytype_datatype_t dt,
                                         TcConfigMode cfg_mode) {
  if (dt != UPLL_DT_CANDIDATE) {
    return true;
  }

  std::map<unc_key_type_t, UpllKtView>::iterator view_it = kt_view_.find(kt);
  if (view_it == kt_view_.end()) {
    return false;
  }

  if (cfg_mode == TC_CONFIG_GLOBAL) {
    return true;
  } else {
    UpllKtView kt_view = view_it->second;

    if ((cfg_mode == TC_CONFIG_VIRTUAL) && (kt_view & kUpllKtViewVirtual))
     return true;
    else if ((cfg_mode == TC_CONFIG_VTN) && (kt_view & kUpllKtViewVtn))
      return true;
    return false;
  }
  return false;
}


upll_rc_t UpllConfigMgr::ValidSession(uint32_t clnt_sess_id,
                                      uint32_t config_id,
                                      TcConfigMode config_mode,
                                      std::string vtn_name) {
  UPLL_FUNC_TRACE;
  unc::tclib::TcLibModule *tclib = GetTcLibModule();
  if (!tclib)
    return UPLL_RC_ERR_GENERIC;

  uint32_t err = tclib->TcLibValidateUpdateMsg(clnt_sess_id, config_id,
                                               config_mode, vtn_name);
  if (unc::tclib::TC_API_COMMON_SUCCESS == err) {
    return UPLL_RC_SUCCESS;
  }

  if (err == unc::tclib::TC_INVALID_SESSION_ID) {
      UPLL_LOG_DEBUG("Invalid session_id %u in IPC request", clnt_sess_id);
  } else if (err == unc::tclib::TC_INVALID_CONFIG_ID) {
      UPLL_LOG_DEBUG("Invalid config_id %u in IPC request", config_id);
  }

  return UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID;
}

upll_rc_t UpllConfigMgr::ValidIpcOption1(unc_keytype_option1_t opt1) {
  switch (opt1) {
    case UNC_OPT1_NORMAL:
    case UNC_OPT1_DETAIL:
    case UNC_OPT1_COUNT:
      return UPLL_RC_SUCCESS;
    default:
      return UPLL_RC_ERR_INVALID_OPTION1;
  }
  return UPLL_RC_ERR_INVALID_OPTION1;
}

upll_rc_t UpllConfigMgr::ValidIpcDatatype(upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;
  switch (datatype) {
    case UPLL_DT_STATE:
    case UPLL_DT_CANDIDATE:
    case UPLL_DT_RUNNING:
    case UPLL_DT_STARTUP:
    case UPLL_DT_IMPORT:
      return UPLL_RC_SUCCESS;
    case UPLL_DT_AUDIT:
    default:
        return UPLL_RC_ERR_NO_SUCH_DATATYPE;
  }
  return UPLL_RC_ERR_NO_SUCH_DATATYPE;
}

upll_rc_t UpllConfigMgr::ValidIpcOperation(upll_service_ids_t service,
                                           upll_keytype_datatype_t datatype,
                                           unc_keytype_operation_t operation) {
  UPLL_FUNC_TRACE;
  if (service == UPLL_EDIT_SVC_ID) {
    if (!IsActiveNode()) {
      UPLL_LOG_WARN("Standby node does not allow EDIT operations");
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
    }
    switch (operation) {
      case UNC_OP_CREATE:
      case UNC_OP_DELETE:
      case UNC_OP_UPDATE:
        if (datatype == UPLL_DT_CANDIDATE) {
          return UPLL_RC_SUCCESS;
        } else {
          return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        }
        break;
      case UNC_OP_RENAME:
        if (datatype == UPLL_DT_IMPORT) {
          return UPLL_RC_SUCCESS;
        } else {
          return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        }
      default:
        return UPLL_RC_ERR_NO_SUCH_OPERATION;
    }
  } else if (service == UPLL_READ_SVC_ID) {
    if (!IsActiveNode()) {
      if (datatype == UPLL_DT_STATE) {
        UPLL_LOG_WARN("Standby node does not allow READ on DT_STATE");
        return UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
      }
    }
    switch (operation) {
      case UNC_OP_READ:
      case UNC_OP_READ_SIBLING:
      case UNC_OP_READ_SIBLING_BEGIN:
        return UPLL_RC_SUCCESS;
      case UNC_OP_READ_SIBLING_COUNT:
        if (datatype == UPLL_DT_CANDIDATE || datatype == UPLL_DT_RUNNING ||
            datatype == UPLL_DT_STARTUP || datatype == UPLL_DT_STATE) {
          return UPLL_RC_SUCCESS;
        } else {
          return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        }
        break;
      case UNC_OP_READ_NEXT:
      case UNC_OP_READ_BULK:
        if (datatype == UPLL_DT_STATE) {
          return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
        }
        return UPLL_RC_SUCCESS;
      default:
        return UPLL_RC_ERR_NO_SUCH_OPERATION;
    }
  } else if (service == UPLL_CONTROL_SVC_ID) {
    if (!IsActiveNode()) {
      UPLL_LOG_WARN("Standby node does not allow CONTROL operations");
      return UPLL_RC_ERR_NOT_SUPPORTED_BY_STANDBY;
    }
    if (operation == UNC_OP_CONTROL)
      return UPLL_RC_SUCCESS;
    else
      return UPLL_RC_ERR_NO_SUCH_OPERATION;
  }
  return UPLL_RC_ERR_BAD_REQUEST;     // Unknown keytype service
}

upll_rc_t UpllConfigMgr::ValidateKtRequest(upll_service_ids_t service,
                                   const IpcReqRespHeader &msghdr,
                                   const ConfigKeyVal &ckv) {
  UPLL_FUNC_TRACE;
  upll_rc_t urc;
  if ((service == UPLL_EDIT_SVC_ID) ||
      (service == UPLL_READ_SVC_ID && msghdr.datatype == UPLL_DT_CANDIDATE)) {
    // Validate session id and config id in the case of edit services and
    // read on candidate configuration.

    TcConfigMode config_mode = TC_CONFIG_INVALID;
    char *vtn_name = NULL;

    // Assume config mode type by the received key types and if the
    // mode type is VTN mode get the vtn name from the corresponding key
    // ASSUME_CONFIG_MODE_TYPE(ckv.get_key_type(), config_mode);
    AssumeConfigModeType(ckv.get_key_type(), config_mode);

    if (config_mode == TC_CONFIG_VTN) {
      if ((msghdr.operation != UNC_OP_READ_SIBLING_BEGIN) &&
          (msghdr.operation != UNC_OP_READ_SIBLING_COUNT) &&
          (msghdr.operation != UNC_OP_READ_BULK)) {
        urc = GetVtnName(msghdr, ckv, &vtn_name);
        if (urc != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("GetVtnName failed");
          if (vtn_name) {
            free(vtn_name);
          }
          return urc;
        }
      }
    }

    std::string vtn_id;
    if (vtn_name) {
      vtn_id.append(vtn_name);
      free(vtn_name);
    }
    UPLL_LOG_DEBUG("session_id %u and/or config_id %u in",
                    msghdr.clnt_sess_id, msghdr.config_id);

    if ((msghdr.operation != UNC_OP_READ_SIBLING_BEGIN) &&
        (msghdr.operation != UNC_OP_READ_SIBLING_COUNT) &&
        (msghdr.operation != UNC_OP_READ_BULK)) {
      urc = ValidSession(msghdr.clnt_sess_id, msghdr.config_id,
                         config_mode, vtn_id);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("Invalid session_id %u and/or config_id %u in"
                       "IPC request Err: %d", msghdr.clnt_sess_id,
                       msghdr.config_id, urc);
        return urc;
      }
    }
  }
  urc = ValidIpcOperation(service, msghdr.datatype, msghdr.operation);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Invalid operation %u in IPC request for service %u",
                  msghdr.operation, service);
    return urc;
  }

  urc = ValidIpcOption1(msghdr.option1);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Invalid option1 %u in IPC request", msghdr.option1);
    return urc;
  }
  if (service == UPLL_EDIT_SVC_ID) {
    // only allowed option combination is normal and none
    if (msghdr.option1 != UNC_OPT1_NORMAL) {
      UPLL_LOG_DEBUG("Invalid option1 %u, expected %d in IPC request",
                    msghdr.option1, UNC_OPT1_NORMAL);
      return UPLL_RC_ERR_INVALID_OPTION1;
    }
    if (msghdr.option2 != UNC_OPT2_NONE) {
      UPLL_LOG_DEBUG("Invalid option2 %u, expected %d in IPC request",
                     msghdr.option1, UNC_OPT2_NONE);
      return UPLL_RC_ERR_INVALID_OPTION2;
    }
  }
  urc = ValidIpcDatatype(msghdr.datatype);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_DEBUG("Invalid datatype %u in IPC request", msghdr.datatype);
    return urc;
  }

  if ((msghdr.rep_count > 0) &&
      (msghdr.operation != UNC_OP_READ_BULK &&
       msghdr.operation != UNC_OP_READ_SIBLING_BEGIN &&
       msghdr.operation != UNC_OP_READ_SIBLING)) {
    UPLL_LOG_WARN("RepCount is ignored in the IPC request for operation %d",
                  msghdr.operation);
  }
  return UPLL_RC_SUCCESS;
}

bool UpllConfigMgr::FindRequiredLocks(unc_keytype_operation_t oper,
                                      upll_keytype_datatype_t datatype,
                                      upll_keytype_datatype_t *lck_dt_1,
                                      ConfigLock::LockType *lck_type_1,
                                      upll_keytype_datatype_t *lck_dt_2,
                                      ConfigLock::LockType *lck_type_2,
                                      TaskPriority *lock_prior) {
  UPLL_FUNC_TRACE;
  *lck_dt_1 = UPLL_DT_INVALID;
  *lck_dt_2 = UPLL_DT_INVALID;
  switch (oper) {
    case UNC_OP_READ:
    case UNC_OP_READ_NEXT:
    case UNC_OP_READ_BULK:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING_COUNT:
      if (datatype == UPLL_DT_AUDIT)
        return false;
      *lck_dt_1 = datatype;
      *lck_type_1 = ConfigLock::CFG_READ_LOCK;
      *lock_prior = kNormalTaskPriority;
      return true;
      break;
    case UNC_OP_CREATE:
    case UNC_OP_UPDATE:
    case UNC_OP_DELETE:
      if (datatype == UPLL_DT_CANDIDATE) {
        *lck_dt_1 = UPLL_DT_CANDIDATE;
        *lck_type_1 = ConfigLock::CFG_WRITE_LOCK;
        *lck_dt_2 = UPLL_DT_RUNNING;
        *lck_type_2 = ConfigLock::CFG_READ_LOCK;
        *lock_prior = kCriticalTaskPriority;
        return true;
      } else if (datatype == UPLL_DT_IMPORT) {
        *lck_dt_1 = UPLL_DT_IMPORT;
        *lck_type_1 = ConfigLock::CFG_WRITE_LOCK;
        *lck_dt_2 = UPLL_DT_RUNNING;
        *lck_type_2 = ConfigLock::CFG_READ_LOCK;
        *lock_prior = kNormalTaskPriority;
        return true;
      } else if (datatype == UPLL_DT_AUDIT) {
        *lck_dt_1 = UPLL_DT_AUDIT;
        *lck_type_1 = ConfigLock::CFG_WRITE_LOCK;
        *lck_dt_2 = UPLL_DT_RUNNING;
        *lck_type_2 = ConfigLock::CFG_READ_LOCK;
        *lock_prior = kNormalTaskPriority;
        return true;
      }
      return false;
      break;
    case UNC_OP_RENAME:
      if (datatype == UPLL_DT_IMPORT) {
        *lck_dt_1 = UPLL_DT_IMPORT;
        *lck_type_1 = ConfigLock::CFG_WRITE_LOCK;
        *lck_dt_2 = UPLL_DT_CANDIDATE;
        *lck_type_2 = ConfigLock::CFG_READ_LOCK;
        *lock_prior = kNormalTaskPriority;
        return true;
      }
      return false;
      break;
    case UNC_OP_CONTROL:
      // Currently Control Operations only read the running configurations
      // So, take read lock on RUNNING.
      if (datatype == UPLL_DT_RUNNING) {
        *lck_dt_1 = datatype;
        *lck_type_1 = ConfigLock::CFG_READ_LOCK;
        *lock_prior = kNormalTaskPriority;
        return true;
      }
      return false;
      break;
    default:
      return false;
  }
  return false;
}

bool UpllConfigMgr::TakeConfigLock(unc_keytype_operation_t oper,
                                   upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;
  upll_keytype_datatype_t lck_dt_1 = UPLL_DT_INVALID;
  upll_keytype_datatype_t lck_dt_2 = UPLL_DT_INVALID;
  ConfigLock::LockType lck_type_1, lck_type_2;
  TaskPriority task_priorirty;

  if (datatype == UPLL_DT_STATE)
    datatype = UPLL_DT_RUNNING;

  if (FindRequiredLocks(oper, datatype,
                        &lck_dt_1, &lck_type_1,
                        &lck_dt_2, &lck_type_2,
                        &task_priorirty)) {
    if (lck_dt_2 == UPLL_DT_INVALID)
      return cfg_lock_.Lock(task_priorirty, lck_dt_1, lck_type_1);
    else
      return cfg_lock_.Lock(task_priorirty, lck_dt_1, lck_type_1,
                            lck_dt_2, lck_type_2);
  }
  return false;
}

bool UpllConfigMgr::ReleaseConfigLock(unc_keytype_operation_t oper,
                                      upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;
  upll_keytype_datatype_t lck_dt_1 = UPLL_DT_INVALID;
  upll_keytype_datatype_t lck_dt_2 = UPLL_DT_INVALID;
  ConfigLock::LockType lck_type_1, lck_type_2;
  TaskPriority task_priorirty;

  if (datatype == UPLL_DT_STATE)
    datatype = UPLL_DT_RUNNING;

  if (FindRequiredLocks(oper, datatype,
                        &lck_dt_1, &lck_type_1,
                        &lck_dt_2, &lck_type_2,
                        &task_priorirty)) {
    if (lck_dt_2 == UPLL_DT_INVALID)
      return cfg_lock_.Unlock(task_priorirty, lck_dt_1);
    else
      return cfg_lock_.Unlock(task_priorirty, lck_dt_1, lck_dt_2);
  }
  return false;
}

upll_rc_t UpllConfigMgr::ReadNextMo(IpcReqRespHeader *req, ConfigKeyVal *ckv,
                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  // Rep Count in READ_NEXT is ignored. On return from the API it is set to 1
  req->rep_count = 1;
  upll_rc_t urc = ReadBulkMo(req, ckv, dmi);

  if (urc != UPLL_RC_SUCCESS) {
    req->rep_count = 0;
  }

  return urc;
}

upll_rc_t UpllConfigMgr::ReadBulkMo(IpcReqRespHeader *reqhdr, ConfigKeyVal *ckv,
                                    DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t urc;

  ConfigKeyVal *resp_ckv = NULL;
  urc = ReadBulkMo(reqhdr, ckv, &resp_ckv, dmi);
  if (urc == UPLL_RC_SUCCESS) {
    if (resp_ckv != NULL) {
      ckv->ResetWith(resp_ckv);
      delete resp_ckv;
    }
    if (reqhdr->rep_count == 0) {
      urc = reqhdr->result_code = UPLL_RC_ERR_NO_SUCH_INSTANCE;
    }
  }

  return urc;
}

upll_rc_t UpllConfigMgr::ValidateImport(uint32_t sess_id, uint32_t config_id,
                                        const char *ctrlr_id,
                                        uint32_t operation,
                                        upll_import_type import_type) {
  UPLL_FUNC_TRACE;

  // fix import_operation string for debug purpose
  const char *import_operation = "";
  if (operation == UPLL_IMPORT_CTRLR_CONFIG_OP)
    import_operation = "import";
  else if (operation == UPLL_MERGE_IMPORT_CONFIG_OP)
    import_operation = "merge";
  else if (operation == UPLL_CLEAR_IMPORT_CONFIG_OP)
    import_operation = "clear";
  else
    return UPLL_RC_ERR_GENERIC;

  // Validate user session
  string vtn_name = "";
  upll_rc_t urc = ValidSession(sess_id, config_id, TC_CONFIG_GLOBAL, vtn_name);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Invalid session_id %u and/or config_id %u in IPC request,"
                  " Err: %d, %s not allowed for controller %s",
                  sess_id, config_id, urc, import_operation, ctrlr_id);
    return urc;
  }

  /* TODO - can contoller be deleted and added with different type when import
   * is in progress? -- Such scenario need to be restricted by Physical. For now
   * Logical does not implement such a check. */

  // Check controller type
  unc_keytype_ctrtype_t ctrlr_type;
  if (false == CtrlrMgr::GetInstance()->GetCtrlrType(
      ctrlr_id, UPLL_DT_CANDIDATE, &ctrlr_type)) {
    UPLL_LOG_INFO("Unable to get controller type. Cannot do %s for %s ",
                  import_operation, ctrlr_id);
    return UPLL_RC_ERR_NO_SUCH_INSTANCE;
  }
  if ((ctrlr_type != UNC_CT_PFC) && (ctrlr_type != UNC_CT_ODC)) {
    UPLL_LOG_INFO("Import is not allowed for controller type %d."
                  " Cannot do %s for %s ",
                  ctrlr_type, import_operation, ctrlr_id);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

  if (operation == UPLL_CLEAR_IMPORT_CONFIG_OP)
    return UPLL_RC_SUCCESS;  // the other below checks are not required
  bool is_audit_wait = false;
  urc = GetControllerSatusFromPhysical(
                      reinterpret_cast<uint8_t *>(const_cast<char *>(ctrlr_id)),
                      is_audit_wait);
  if (UPLL_RC_SUCCESS == urc) {
    if (!is_audit_wait) {
      UPLL_LOG_INFO("Full / Partial Import is not allowed for controller"
                    " %s ",
                    ctrlr_id);
      return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
    }
  } else {
    UPLL_LOG_INFO("GetControllerStatusFromPhysical function failed");
    return urc;
  }
  if (UPLL_RC_SUCCESS == urc &&
      UPLL_IMPORT_TYPE_PARTIAL == import_type) {
    bool audit_type = false;
    CtrlrMgr *ctrlr_mgr = CtrlrMgr::GetInstance();
    urc = ctrlr_mgr->GetAuditType(ctrlr_id,
                                  UPLL_DT_RUNNING,
                                  &audit_type);
    if (UPLL_RC_SUCCESS != urc) {
      UPLL_LOG_DEBUG("GetAuditType failed %d", urc);
      return urc;
    }
    if (audit_type) {
      UPLL_LOG_INFO("Partial Import is not allowed for controller"
                    " %s ",
                     ctrlr_id);
      return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
    }
  }

  bool candidate_dirty = true;
  // Note: caller supposed to take the lock on the database
  urc = IsCandidateDirtyNoLock(TC_CONFIG_GLOBAL, vtn_name, &candidate_dirty,
                               false);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to find if candidate is dirty. Urc=%d."
                  " Cannot do %s for %s", urc, import_operation, ctrlr_id);
    return urc;
  }
  if (candidate_dirty == true) {
    UPLL_LOG_INFO("There are uncommitted changes. Cannot do %s for %s",
                  import_operation, ctrlr_id);
    return UPLL_RC_ERR_CANDIDATE_IS_DIRTY;
  }

  bool ctr_in_use = true;
  key_ctr_t *ctr_key = reinterpret_cast<key_ctr_t*>(
      ConfigKeyVal::Malloc(sizeof(key_ctr_t)));
  upll_strncpy(ctr_key->controller_name, ctrlr_id, KtUtil::kCtrlrNameLenWith0);
  ConfigKeyVal ckv(UNC_KT_CONTROLLER, IpctSt::kIpcStKeyCtr, ctr_key);
  // Note: Partial import need not to check the controller in use.
  if (UPLL_IMPORT_TYPE_PARTIAL != import_type) {
    // Note: caller supposed to take the lock on the database
    urc = IsKeyInUseNoLock(UPLL_DT_RUNNING, &ckv, &ctr_in_use);
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Failed to find if controller is in running config. Urc=%d."
                    "Cannot do %s for %s", urc, import_operation, ctrlr_id);
      return urc;
    }
    if (ctr_in_use == true) {
      return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
    }
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllConfigMgr::ImportCtrlrConfig(const char *ctrlr_id,
                                           upll_keytype_datatype_t dest_dt,
                                           upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = UPLL_RC_SUCCESS;
  DalOdbcMgr *dom = NULL;
  // This method would be invoked for both Import and audit.
  // So, the connections should be managed appropriately.
  if (UPLL_DT_IMPORT == dest_dt) {
    dom = dbcm_->GetConfigRwConn();
  } else if (UPLL_DT_AUDIT == dest_dt) {
    dom = dbcm_->GetAuditRwConn();
  } else {
    UPLL_LOG_FATAL("It should not be here.");
  }
  if (dom == NULL) { return UPLL_RC_ERR_GENERIC; }

  key_root_t *root_key = reinterpret_cast<key_root_t *>(
      ConfigKeyVal::Malloc(sizeof(key_root_t)));


  ConfigKeyVal *bulkreq_ckv = new ConfigKeyVal(UNC_KT_ROOT,
                                               IpctSt::kIpcStKeyRoot,
                                               root_key);

  bool read_bulk_done = false;

  if (UPLL_IMPORT_TYPE_PARTIAL == import_type) {
    unc_key_type_t rename_key[]= { UNC_KT_VBRIDGE, UNC_KT_VLINK,
                                   UNC_KT_FLOWLIST, UNC_KT_POLICING_PROFILE };

    for (unsigned int i = 0; i < sizeof(rename_key)/sizeof(rename_key[0]);
         i++) {
      std::map<unc_key_type_t, MoManager*>::iterator momgr_it =
          upll_kt_momgrs_.find(rename_key[i]);
      if (momgr_it != upll_kt_momgrs_.end()) {
        MoManager *momgr = momgr_it->second;
        urc = momgr->CopyRenameTables(ctrlr_id, dest_dt, dom);
      }
      if (UPLL_RC_SUCCESS != urc)
        break;
    }
     // Checks wheather CopyRenameTable function is success or failed
    if (UPLL_RC_SUCCESS != urc) {
      UPLL_LOG_ERROR("CopyRenameTables failed");
      DELETE_IF_NOT_NULL(bulkreq_ckv);
      dbcm_->DalTxClose(dom, false);
      dbcm_->ReleaseRwConn(dom);
      return urc;
    } else {
      // continue
    }
  }

  //  During import operation copy vtn_unified information from candidate
  //  to import, to apply renamed vtn_name to vtn_unified_tbl
  if (dest_dt == UPLL_DT_IMPORT) {
    std::map<unc_key_type_t, MoManager*>::iterator momgr_it =
        upll_kt_momgrs_.find(UNC_KT_VTN_UNIFIED);
    if (momgr_it != upll_kt_momgrs_.end()) {
      MoManager *momgr = momgr_it->second;
      urc = momgr->CopyVtnUnifiedTable(dest_dt, dom);
    }
     // Checks whether CopyVtnUnifiedTable function is success or failed
    if (UPLL_RC_SUCCESS != urc) {
      UPLL_LOG_ERROR("CopyVtnUnified failed");
      DELETE_IF_NOT_NULL(bulkreq_ckv);
      dbcm_->DalTxClose(dom, false);
      dbcm_->ReleaseRwConn(dom);
      return urc;
    }
  }

  while (!read_bulk_done) {
    if (dest_dt == UPLL_DT_AUDIT) {
      urc = ContinueAuditProcess();
    } else {
      urc = ContinueActiveProcess();
    }
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Import to DT %d aborted, Urc=%d", dest_dt, urc);
      read_bulk_done = true;
      break;  // break from inner loop
    }
    IpcRequest bulk_req;
    IpcResponse bulk_resp;
    bzero(&bulk_req, sizeof(IpcRequest));
    bzero(&bulk_resp, sizeof(IpcResponse));

    // bulk_req.header.clnt_sess_id = 0;
    // bulk_req.header.config_id = 0;
    bulk_req.header.operation = UNC_OP_READ_BULK;
    bulk_req.header.rep_count = 0xFFFFFFFF;  // Driver might be ignoring this
    // Inform driver that read_bulk is sent for audit
    // to perform required processing while cancel audit
    if (dest_dt == UPLL_DT_AUDIT) {
      bulk_req.header.option1 = UNC_OPT1_AUDIT;
    } else {
      bulk_req.header.option1 = UNC_OPT1_NORMAL;
    }
    bulk_req.header.option2 = UNC_OPT2_NONE;
    bulk_req.header.datatype = UPLL_DT_RUNNING;
    bulk_req.ckv_data = bulkreq_ckv;
    if (IpcUtil::SendReqToDriver(ctrlr_id, NULL, NULL, 0,
                                 &bulk_req, false, &bulk_resp) == false) {
      UPLL_LOG_ERROR("Failed to send request to driver for %s", ctrlr_id);
      urc = bulk_resp.header.result_code;
      if (bulk_req.ckv_data) {
        delete bulk_req.ckv_data;
        bulk_req.ckv_data = NULL;
        bulkreq_ckv = NULL;
      }
      if (bulk_resp.ckv_data) {
        delete bulk_resp.ckv_data;
        bulk_resp.ckv_data = NULL;
      }
      read_bulk_done = true;
      break;    // Done with READBULK
    }

    delete bulk_req.ckv_data;
    bulk_req.ckv_data = NULL;
    bulkreq_ckv = NULL;

    if (bulk_resp.header.result_code == UPLL_RC_ERR_NO_SUCH_INSTANCE) {
      // No more elements in the controller configuration.
      UPLL_LOG_TRACE("No more elements in the controller configuration");
      urc = UPLL_RC_SUCCESS;
      if (bulk_resp.ckv_data) {
        delete bulk_resp.ckv_data;
        bulk_resp.ckv_data = NULL;
      }
      read_bulk_done = true;
      break;    // Done with READBULK
    } else if (bulk_resp.header.result_code != UPLL_RC_SUCCESS) {
      urc = bulk_resp.header.result_code;
      if (bulk_resp.ckv_data) {
        delete bulk_resp.ckv_data;
        bulk_resp.ckv_data = NULL;
      }
      read_bulk_done = true;
      break;
    }

    // Process the bulk respose and send another READ_BULK request with last
    // CKV in the response
    ConfigKeyVal *one_ckv = NULL;
    // Create each KT instance at a time in the destination DT
    while (bulk_resp.ckv_data) {
      if (dest_dt == UPLL_DT_AUDIT) {
        urc = ContinueAuditProcess();
      } else {
        urc = ContinueActiveProcess();
      }
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Import to DT %d aborted, Urc=%d", dest_dt, urc);
        delete bulk_resp.ckv_data;
        bulk_resp.ckv_data = NULL;
        read_bulk_done = true;
        break;  // break from inner loop
      }
      one_ckv = bulk_resp.ckv_data;
      bulk_resp.ckv_data = one_ckv->get_next_cfg_key_val();
      one_ckv->set_next_cfg_key_val(NULL);

      IpcReqRespHeader one_reqhdr;
      bzero(&one_reqhdr, sizeof(IpcReqRespHeader));
      // clnt_sess_id, config_id, rep_count, option1, option2 = 0
      one_reqhdr.operation = UNC_OP_CREATE;
      one_reqhdr.datatype = dest_dt;

      unc_key_type_t kt = one_ckv->get_key_type();
      UPLL_LOG_TRACE("KeyType in Driver message: %u", kt);
      std::map<unc_key_type_t, MoManager*>::iterator momgr_it =
        upll_kt_momgrs_.find(kt);
      if (momgr_it != upll_kt_momgrs_.end()) {
        MoManager *momgr = momgr_it->second;
        // In import-readbulk driver cannot return domain id in message header
        // as there will be more than one VTN. A VTN will be sent in the
        // response only if a VNODE is present for the VTN. VNODE val structure
        // will have domain id.
        if (dest_dt == UPLL_DT_AUDIT) {
          // Rename and autrename is supported only for PFC keytypes.
          // If controller type is other than PFC, CreateImportMo should be
          // invoked with UPLL_IMPORT_TYPE_FULL import type.
          // So normal CreateAuditMoImpl will be invoked.
          unc_keytype_ctrtype_t ctrlr_type;
          if (false == CtrlrMgr::GetInstance()->GetCtrlrType(
                  ctrlr_id, UPLL_DT_RUNNING, &ctrlr_type)) {
            UPLL_LOG_INFO("Unable to get controller type for %s ",
                          ctrlr_id);
            delete one_ckv;
            one_ckv = NULL;
            if (bulk_resp.ckv_data) {
              delete bulk_resp.ckv_data;
              bulk_resp.ckv_data = NULL;
            }
            read_bulk_done = true;
            urc = UPLL_RC_ERR_GENERIC;
            break;  // break from inner loop
          }
          if ((ctrlr_type != UNC_CT_PFC) && (ctrlr_type != UNC_CT_ODC)) {
            urc = momgr->CreateImportMo(&one_reqhdr, one_ckv, dom,
                                        ctrlr_id, NULL, UPLL_IMPORT_TYPE_FULL);
          } else {
            urc = momgr->CreateImportMo(&one_reqhdr, one_ckv, dom,
                                        ctrlr_id, NULL, import_type);
          }
        } else {
          urc = momgr->CreateImportMo(&one_reqhdr, one_ckv, dom,
                                      ctrlr_id, NULL, import_type);
        }

        delete one_ckv;
        one_ckv = NULL;
        if (urc != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("CreateImportMo failed. Error = %d", urc);
          if (bulk_resp.ckv_data) {
            delete bulk_resp.ckv_data;
            bulk_resp.ckv_data = NULL;
          }
          read_bulk_done = true;
          break;  // break from inner loop
        }
      }

      /* Enable the below code if Driver can support READ_BULK in chunks:
      // Need to preserve the last CKV for continuing READ_BULK operation to
      // remaining data
      if (bulk_resp.ckv_data != NULL) {
        delete one_ckv;
      } else {
        // install last CKV from the bulk response into READ_BULK next request
        bulkreq_ckv = one_ckv;
      }
      */
    }

    // Response processing completed
    if (urc != UPLL_RC_SUCCESS) {
      read_bulk_done = true;
      break;    // exit from outer while loop
    }

    // Modify the below line if Driver can support READ_BULK in chunks
    read_bulk_done = true;
  }

  if (urc == UPLL_RC_SUCCESS &&
      (UPLL_DT_IMPORT == dest_dt || UPLL_DT_AUDIT == dest_dt)&&
      UPLL_IMPORT_TYPE_PARTIAL == import_type) {
    if (dest_dt == UPLL_DT_IMPORT) {
      unc_key_type_t rename_key[]= { UNC_KT_VBRIDGE,
        UNC_KT_VLINK, UNC_KT_FLOWLIST, UNC_KT_POLICING_PROFILE };
      for (unsigned int i = 0; i < sizeof(rename_key)/sizeof(rename_key[0]);
           i++) {
        std::map<unc_key_type_t, MoManager*>::iterator momgr_it =
            upll_kt_momgrs_.find(rename_key[i]);
        if (momgr_it != upll_kt_momgrs_.end()) {
          MoManager *momgr = momgr_it->second;
          urc = momgr->PurgeRenameTable(rename_key[i], ctrlr_id, dom);
        }
        if (UPLL_RC_SUCCESS != urc)
          break;
      }
    }
    /*
     * The redirect node may be auto renamed during partial import operation
     * So redirect noeds need to convert PFC to UNC after create operations
     * of all keytpes
     */
    unc_key_type_t redirect_key[]= { UNC_KT_VBR_FLOWFILTER_ENTRY,
      UNC_KT_VBRIF_FLOWFILTER_ENTRY, UNC_KT_VRTIF_FLOWFILTER_ENTRY,
      UNC_KT_VTERMIF_FLOWFILTER_ENTRY };
    for (unsigned int i = 0; i < sizeof(redirect_key)/sizeof(redirect_key[0]);
        i++) {
      std::map<unc_key_type_t, MoManager*>::iterator momgr_it =
        upll_kt_momgrs_.find(redirect_key[i]);
      if (momgr_it != upll_kt_momgrs_.end()) {
        MoManager *momgr = momgr_it->second;
        urc = momgr->GetRenamedUncKeyWithRedirection(redirect_key[i], dest_dt,
                                                     ctrlr_id, dom);
      }
      if (UPLL_RC_SUCCESS != urc)
        break;
    }
  }

  UPLL_LOG_DEBUG("Import configuration status. Urc=%d", urc);
  DELETE_IF_NOT_NULL(bulkreq_ckv);

  upll_rc_t close_urc = dbcm_->DalTxClose(dom, (urc == UPLL_RC_SUCCESS));
  dbcm_->ReleaseRwConn(dom);

  if (urc == UPLL_RC_SUCCESS) {
    urc = close_urc;
  }

  return urc;
}

upll_rc_t UpllConfigMgr::StartImport(const char *ctrlr_id,
                                     uint32_t sess_id,
                                     uint32_t config_id,
                                     upll_import_type in_import_type) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("ctrlr_id: %s, sess_id: %d, config_id: %d, import_type: %d",
                ctrlr_id, sess_id, config_id, in_import_type);
  upll_rc_t urc;

  // If in BATCH mode perform DB Commit.
  urc = DbCommitIfInBatchMode(__FUNCTION__, kNormalTaskPriority);
  if (urc != UPLL_RC_SUCCESS) {
    return urc;
  }

  pfc::core::ScopedMutex import_lock(import_mutex_lock_);

  // Note: Checks the import type is partial or full
  if (in_import_type != UPLL_IMPORT_TYPE_FULL &&
      in_import_type != UPLL_IMPORT_TYPE_PARTIAL) {
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  // Note: Check the import initial state
  if (import_state_progress == 0) {
    // Note: In initial state assign the state
    import_type = current_import_type = in_import_type;
    import_state_progress++;
  } else {
    // Note: if not initial state checks same import type or not
    if (import_type == current_import_type) {
      // do nothing
    } else {
      return UPLL_RC_ERR_BAD_REQUEST;
    }
  }

  // Validate if import is already in progress
  if (import_in_progress_ == true) {
    UPLL_LOG_INFO("Import is in progress for %s. Cannot do import for %s",
                   import_ctrlr_id_.c_str(), ctrlr_id);
    import_state_progress = 0;
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }

  // Take read lock on running for doing IsCandidateDirty and IsKeyInUse check
  // in ValidateImport
  ScopedConfigLock scfg_lock(cfg_lock_, kNormalTaskPriority,
                             UPLL_DT_IMPORT, ConfigLock::CFG_WRITE_LOCK,
                             UPLL_DT_CANDIDATE, ConfigLock::CFG_READ_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  urc = ValidateImport(sess_id, config_id, ctrlr_id,
                       UPLL_IMPORT_CTRLR_CONFIG_OP,
                       import_type);
  if (urc != UPLL_RC_SUCCESS) {
    import_state_progress = 0;
    return urc;
  }

  // All checks are over. Let us import configuration.
  import_in_progress_ = true;
  import_ctrlr_id_ = ctrlr_id;

  urc = ImportCtrlrConfig(ctrlr_id, UPLL_DT_IMPORT, import_type);

  if (urc != UPLL_RC_SUCCESS) {
    import_in_progress_ = false;
    import_ctrlr_id_ = "";
    import_state_progress = 0;
  }

  return urc;
}

upll_rc_t UpllConfigMgr::OnMerge(uint32_t sess_id, uint32_t config_id,
                                 upll_import_type in_import_type) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("sess_id: %d, config_id: %d, import_type: %d",
                sess_id, config_id, in_import_type);
  upll_rc_t urc;

  // If in BATCH mode perform DB Commit.
  urc = DbCommitIfInBatchMode(__FUNCTION__, kNormalTaskPriority);
  if (urc != UPLL_RC_SUCCESS) {
    return urc;
  }

  // Validate if import is in progress
  pfc::core::ScopedMutex lock(import_mutex_lock_);

  // Note: Checks the import type is partial or full
  if (in_import_type != UPLL_IMPORT_TYPE_FULL &&
      in_import_type != UPLL_IMPORT_TYPE_PARTIAL) {
    return UPLL_RC_ERR_BAD_REQUEST;
  }
  // Note: Check the import initial state
  if (import_state_progress == 0) {
    // Note: In initial state assign the state
    import_type = current_import_type = in_import_type;
    import_state_progress++;
  } else {
    // Note: if not initial state checks same import type or not
    if (import_type == current_import_type) {
      // do nothing
    } else {
      return UPLL_RC_ERR_BAD_REQUEST;
    }
  }

  if (import_in_progress_ == false) {
    UPLL_LOG_INFO("Import is not in progress. Merge cannot be done");
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }

  // Take read lock on running for doing IsCandidateDirty and IsKeyInUse check
  // in ValidateImport
  ScopedConfigLock *scfg_lock = NULL;

  if (UPLL_IMPORT_TYPE_FULL == import_type) {
    scfg_lock = new ScopedConfigLock(cfg_lock_, kNormalTaskPriority,
                                     UPLL_DT_IMPORT, ConfigLock::CFG_READ_LOCK,
                                     UPLL_DT_CANDIDATE,
                                     ConfigLock::CFG_WRITE_LOCK,
                                     UPLL_DT_RUNNING,
                                     ConfigLock::CFG_READ_LOCK);
  } else {
    scfg_lock = new ScopedConfigLock(cfg_lock_, kNormalTaskPriority,
                                     UPLL_DT_IMPORT, ConfigLock::CFG_WRITE_LOCK,
                                     UPLL_DT_CANDIDATE,
                                     ConfigLock::CFG_WRITE_LOCK,
                                     UPLL_DT_RUNNING,
                                     ConfigLock::CFG_READ_LOCK);
  }

  urc = ValidateImport(sess_id, config_id, import_ctrlr_id_.c_str(),
                       UPLL_MERGE_IMPORT_CONFIG_OP, import_type);

  if (urc == UPLL_RC_SUCCESS) {
    // VTN is created during merge validate
    // Because should not update the description presents
    // in candidate
    // validate if merge conflicts with existing config
    urc = MergeValidate(import_type);
  }

  // merge import config to candidate config
  if (urc == UPLL_RC_SUCCESS) {
    urc = MergeImportToCandidate(import_type);
  }
  // During partial import newly created vtn from controller
  // will get deleted  if purgecandidate is called before
  // merge import to candidate function.

  DalOdbcMgr *dom = dbcm_->GetConfigRwConn();
  if (dom == NULL) {
    delete scfg_lock;
    return UPLL_RC_ERR_GENERIC;
  }

  if (UPLL_RC_SUCCESS == urc &&
      UPLL_IMPORT_TYPE_PARTIAL == import_type) {
    urc = PurgeCandidate(dom);
  }

  upll_rc_t db_urc = dbcm_->DalTxClose(dom, (urc == UPLL_RC_SUCCESS));
  dbcm_->ReleaseRwConn(dom);
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }

  delete scfg_lock;
  return urc;
}

upll_rc_t UpllConfigMgr::MergeValidate(upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("import_type: %d", import_type);
  upll_rc_t urc = UPLL_RC_SUCCESS;
  unc_key_type_t kt;

  DalOdbcMgr *dom = dbcm_->GetConfigRwConn();
  if (dom == NULL) { return UPLL_RC_ERR_GENERIC; }

  const std::list<unc_key_type_t> *pre_list = cktt_.get_preorder_list();
  for (std::list<unc_key_type_t>::const_iterator pre_it = pre_list->begin();
       pre_it != pre_list->end(); pre_it++) {
    kt = *pre_it;
    bool flag = false;
    std::map<unc_key_type_t, MoManager*>::iterator momgr_it =
      upll_kt_momgrs_.find(kt);
    if (momgr_it != upll_kt_momgrs_.end()) {
       unc_key_type_t child_key[]= { UNC_KT_VBRIDGE, UNC_KT_VBR_VLANMAP,
                    UNC_KT_VBR_NWMONITOR, UNC_KT_VBR_NWMONITOR_HOST,
                    UNC_KT_VBR_IF, UNC_KT_VROUTER, UNC_KT_VRT_IPROUTE,
                    UNC_KT_DHCPRELAY_SERVER, UNC_KT_DHCPRELAY_IF,
                    UNC_KT_VRT_IF, UNC_KT_VTERMINAL, UNC_KT_VTERM_IF,
                    UNC_KT_VUNKNOWN, UNC_KT_VUNK_IF,
                    UNC_KT_VTEP, UNC_KT_VTEP_IF, UNC_KT_VTEP_GRP,
                    UNC_KT_VTEP_GRP_MEMBER, UNC_KT_VTUNNEL,
                    UNC_KT_VTUNNEL_IF, UNC_KT_VLINK, UNC_KT_VBR_PORTMAP
       };
       for (unsigned int i = 0; i < sizeof(child_key)/sizeof(child_key[0]);
            i++) {
         const unc_key_type_t ktype = child_key[i];
         if (ktype == kt)  {
           UPLL_LOG_TRACE("Skip MergeValidate Here, Its done in UNC_KT_VTN for"
               "the key type %d if applicable", ktype);
           flag = true;
           break;
         }
       }
       if (flag)
         continue;
/*
      if ((urc = ContinueActiveProcess()) != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Aborting MergeValidate, Urc=%d", urc);
        break;
      }
*/
      MoManager *momgr = momgr_it->second;
      ConfigKeyVal conflict_ckv(kt);
      urc = momgr->MergeValidate(kt, import_ctrlr_id_.c_str(),
                                 &conflict_ckv, dom, import_type);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("MergeValidate failed %d %d", urc, kt);
        break;
      }
    }
  }
  dbcm_->ReleaseRwConn(dom);
  return urc;
}

upll_rc_t UpllConfigMgr::MergeImportToCandidate(upll_import_type import_type) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("import_type: %d", import_type);
  upll_rc_t urc = UPLL_RC_SUCCESS;
  unc_key_type_t kt;

  DalOdbcMgr *dom = dbcm_->GetConfigRwConn();
  if (dom == NULL) { return UPLL_RC_ERR_GENERIC; }

  const std::list<unc_key_type_t> *pre_list = cktt_.get_preorder_list();
  for (std::list<unc_key_type_t>::const_iterator pre_it = pre_list->begin();
       pre_it != pre_list->end(); pre_it++) {
    kt = *pre_it;
    std::map<unc_key_type_t, MoManager*>::iterator momgr_it =
      upll_kt_momgrs_.find(kt);
    if (momgr_it != upll_kt_momgrs_.end()) {
      MoManager *momgr = momgr_it->second;
      urc = momgr->MergeImportToCandidate(kt, import_ctrlr_id_.c_str(), dom,
                                          import_type);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("MergeImportToCandidate failed %d", urc);
        break;
      }
/*
      if ((urc = ContinueActiveProcess()) != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("Aborting MergeValidate, Urc=%d", urc);
        break;
      }
*/
    }
  }
  dbcm_->ReleaseRwConn(dom);
  return urc;
}

// This function does shallow check
upll_rc_t UpllConfigMgr::IsCandidateDirtyShallow(TcConfigMode cfg_mode,
                                                 std::string cfg_vtn_name,
                                                 bool *dirty) {
  UPLL_FUNC_TRACE;
  if (dirty == NULL) {
    UPLL_LOG_DEBUG("Null argument: dirty");
    return UPLL_RC_ERR_GENERIC;
  }
  return IsCandidateDirty(cfg_mode, cfg_vtn_name, dirty);
  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllConfigMgr::IsCandidateDirty(TcConfigMode cfg_mode,
                                          std::string cfg_vtn_name,
                                          bool *dirty) {
  UPLL_FUNC_TRACE;

  ScopedConfigLock scfg_lock(cfg_lock_, kNormalTaskPriority,
                             UPLL_DT_CANDIDATE, ConfigLock::CFG_READ_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);
  // For external API always do shallow check.
  return IsCandidateDirtyNoLock(cfg_mode, cfg_vtn_name, dirty, true);
}

// For PCM, allways shallow check is done
upll_rc_t UpllConfigMgr::IsCandidateDirtyNoLock(
    TcConfigMode cfg_mode, std::string cfg_vtn_name,
    bool *dirty, bool shallow_check) {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = UPLL_RC_SUCCESS;
  unc_key_type_t kt;
  if (dirty == NULL) {
    UPLL_LOG_DEBUG("Null argument: dirty");
    return UPLL_RC_ERR_GENERIC;
  }

  DalOdbcMgr *dom = NULL;
  dom = dbcm_->GetConfigRwConn();

  if (dom == NULL) { return UPLL_RC_ERR_GENERIC; }

  *dirty = false;

  const std::list<unc_key_type_t> *pre_list = cktt_.get_preorder_list();
  for (std::list<unc_key_type_t>::const_iterator pre_it = pre_list->begin();
       pre_it != pre_list->end(); pre_it++) {
    kt = *pre_it;
    std::map<unc_key_type_t, MoManager*>::iterator momgr_it =
      upll_kt_momgrs_.find(kt);
    if (momgr_it != upll_kt_momgrs_.end()) {
      MoManager *momgr = momgr_it->second;
      if (cfg_mode == TC_CONFIG_GLOBAL) {
        urc = momgr->IsCandidateDirtyInGlobal(kt, dirty, dom, shallow_check);
      } else {
        urc = momgr->IsCandidateDirtyShallowInPcm(kt, cfg_mode, cfg_vtn_name,
                                                  dirty, dom);
      }
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("IsCandidateDirty failed %d", urc);
        break;
      }
      if (*dirty == true) {
        UPLL_LOG_DEBUG("CandidateDirty KT - %s", kt_name_map_[kt].c_str());
        break;
      }
    }
  }

  batch_mutex_lock_.lock();
  if (batch_mode_in_progress_) {
    // Only read is performed.
    // IF (batch_mode_in_progress_) Neither commit nor rollback.
    dbcm_->ReleaseRwConn(dom);
  } else {
    upll_rc_t db_urc = dbcm_->DalTxClose(dom, false);
    if (urc == UPLL_RC_SUCCESS) {
      urc = db_urc;
    }
    dbcm_->ReleaseRwConn(dom);
  }
  batch_mutex_lock_.unlock();

  return urc;
}

upll_rc_t UpllConfigMgr::IsKeyInUse(upll_keytype_datatype_t datatype,
                                    const ConfigKeyVal *ckv, bool *in_use) {
  UPLL_FUNC_TRACE;
  // IsKeyInUse is considered as critical task
  ScopedConfigLock scfg_lock(cfg_lock_, kCriticalTaskPriority,
                             datatype, ConfigLock::CFG_READ_LOCK);
  return IsKeyInUseNoLock(datatype, ckv, in_use);
}


upll_rc_t UpllConfigMgr::IsKeyInUseNoLock(upll_keytype_datatype_t datatype,
                                          const ConfigKeyVal *ckv,
                                          bool *in_use) {
  UPLL_FUNC_TRACE;

  PFC_ASSERT(ckv != NULL);
  PFC_ASSERT(in_use != NULL);
  if (ckv == NULL || in_use == NULL) {
    UPLL_LOG_DEBUG("Null argument ckv=%p in_use=%p", ckv, in_use);
    return UPLL_RC_ERR_GENERIC;
  }

  if (datatype != UPLL_DT_CANDIDATE && datatype != UPLL_DT_RUNNING) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_DT;
  }

  upll_rc_t urc;
  switch (ckv->get_key_type()) {
    case UNC_KT_CONTROLLER:
      // set in_use as true if received ctr exists in vtn_ctrlr tbl
      urc = IsKeyInUseNoLock(datatype, ckv, in_use, UNC_KT_VTN);
      if ((urc != UPLL_RC_SUCCESS) || (*in_use == true)) {
        break;
      }

      // set in_use as true if received ctr exists in vbr_portmap_tbl
      urc = IsKeyInUseNoLock(datatype, ckv, in_use, UNC_KT_VBR_PORTMAP);
      if ((urc != UPLL_RC_SUCCESS) || (*in_use == true)) {
        break;
      }

      // If ctr does not exist in vtn_ctrlr_tbl, then do same
      // check in unified_nw_spine_tbl. If exists, set in_use to true
      urc = IsKeyInUseNoLock(datatype, ckv, in_use,
                             UNC_KT_UNW_SPINE_DOMAIN);

      break;
    case UNC_KT_BOUNDARY:
      urc = IsKeyInUseNoLock(datatype, ckv, in_use, UNC_KT_VLINK);
      break;
    case UNC_KT_PORT:
    default:
      return UPLL_RC_ERR_NO_SUCH_NAME;
  }

  return urc;
}

upll_rc_t UpllConfigMgr::IsKeyInUseNoLock(upll_keytype_datatype_t datatype,
                                          const ConfigKeyVal *ckv,
                                          bool *in_use,
                                          unc_key_type_t keytype) {
  UPLL_FUNC_TRACE;

  upll_rc_t urc = UPLL_RC_SUCCESS;
  MoManager *momgr =  GetMoManager(keytype);

  if (momgr) {
    DalOdbcMgr *dom = NULL;

    batch_mutex_lock_.lock();
    if ((UPLL_DT_CANDIDATE == datatype) && batch_mode_in_progress_) {
      dom = dbcm_->GetConfigRwConn();
    } else {
      if (UPLL_RC_SUCCESS != (urc = dbcm_->AcquireRoConn(&dom))) {
         batch_mutex_lock_.unlock();
        return urc;
      }
    }
    batch_mutex_lock_.unlock();

    if (dom == NULL) {
      return UPLL_RC_ERR_GENERIC;
    }

    urc = momgr->IsKeyInUse(datatype, ckv, in_use, dom);

    if (dom->get_conn_type() == uudal::kDalConnReadWrite) {
      // RW connection is taken only if batch_mode_in_progress_ and only read
      // is performed. Neither commit nor rollback.
      dbcm_->ReleaseRwConn(dom);
    } else {
      upll_rc_t db_urc = dbcm_->DalTxClose(dom, false);
      if (urc == UPLL_RC_SUCCESS) {
        urc = db_urc;
      }
      dbcm_->ReleaseRoConn(dom);
    }
  } else {
    urc = UPLL_RC_ERR_GENERIC;
  }

  return urc;
}

void UpllConfigMgr::OnControllerStatusChange(const char *ctrlr_name,
                                             uint8_t operstatus) {
  UPLL_FUNC_TRACE;
  CtrlrMgr *ctr_mgr = CtrlrMgr::GetInstance();
  if (operstatus == UPPL_CONTROLLER_OPER_EVENTS_START) {
    ctr_mgr->AddCtrToDisconnectList(ctrlr_name);
    if (ctr_mgr->IsPathFaultOccured(ctrlr_name, "*")) {
      ctr_mgr->AddCtrToIgnoreList(ctrlr_name);
      OnPathFaultAlarm(ctrlr_name, "*", false);
      ctr_mgr->ClearPathfault(ctrlr_name, "*");
      SendPathFaultAlarm(ctrlr_name, "*", "*", UPLL_CLEAR_WITH_TRAP);
      ctr_mgr->RemoveCtrlrFromIgnoreList(ctrlr_name);
    }
  }
  if (operstatus == UPPL_CONTROLLER_OPER_DOWN ||
      operstatus == UPPL_CONTROLLER_OPER_EVENTS_MERGED) {
    uuk::VtnMoMgr *mgr = reinterpret_cast<uuk::VtnMoMgr *>
      (GetMoManager(UNC_KT_VTN));
    if (mgr != NULL) {
      upll_rc_t urc;
      UPLL_LOG_INFO("Controller(%s) operstatus start : event(%d)",
                     ctrlr_name, operstatus);
      urc = mgr->ControllerStatusHandler(reinterpret_cast<uint8_t*>(
              const_cast<char*>(ctrlr_name)), operstatus, dbcm_, &cfg_lock_);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Failed to update ControllerStatus. "
                       "ctrlr_name=%s operstatus=%d urc=%d",
                       ctrlr_name, operstatus, urc);
      }
      UPLL_LOG_INFO("Controller(%s) operstatus end : event(%d)",
                    ctrlr_name, operstatus);
    }
    if (operstatus == UPPL_CONTROLLER_OPER_EVENTS_MERGED) {
      ctr_mgr->DeleteFromUnknownCtrlrList(ctrlr_name);
      ctr_mgr->RemoveCtrFromDisconnectList(ctrlr_name);
    } else if (operstatus == UPPL_CONTROLLER_OPER_DOWN) {
      ctr_mgr->AddtoUnknownCtrlrList(ctrlr_name);
      ctr_mgr->AddCtrToDisconnectList(ctrlr_name);
      // clear path fault alarm if occurred
      if (ctr_mgr->IsPathFaultOccured(ctrlr_name, "*")) {
        SendPathFaultAlarm(ctrlr_name, "*", "*", UPLL_CLEAR_WITH_TRAP);
      }
    }
  }
}

// NOTE: The functions invoked by this function, access GetAlarmConn(), and
// also take ConfigLock::CFG_READ_LOCK on UPLL_DT_RUNNING
void UpllConfigMgr::OnLogicalPortStatusChange(const char *ctrlr_name,
                                              const char *domain_name,
                                              const char *logical_port_id,
                                              uint8_t oper_status) {
  UPLL_FUNC_TRACE;
  if (toupper(logical_port_id[0]) == 'S' &&
      (toupper(logical_port_id[1]) == 'W' ||
       toupper(logical_port_id[1]) == 'D')) {
    UPLL_LOG_DEBUG("Port event need not be handled for SW/SD ports");
    return;
  }
  CtrlrMgr* ctr_mgr = CtrlrMgr::GetInstance();
  if (ctr_mgr->IsCtrDisconnected(ctrlr_name)) {
    ctr_mgr->AddLogicalPort(ctrlr_name, logical_port_id, oper_status);
    return;
  }
  upll_rc_t   urc = UPLL_RC_SUCCESS;

  if ((toupper(logical_port_id[0]) == 'T' ||
       toupper(logical_port_id[0]) == 'P') &&
       toupper(logical_port_id[1]) == 'P') {
    uuk::VbrIfMoMgr *vbrif_mgr = reinterpret_cast<uuk::VbrIfMoMgr *>
      (GetMoManager(UNC_KT_VBR_IF));
    if (vbrif_mgr != NULL) {
      ConfigKeyVal *vbrif_ckv = NULL;
      urc = vbrif_mgr->GetChildConfigKey(vbrif_ckv, NULL);
      if (urc != UPLL_RC_SUCCESS) {
        return;
      }
      urc = vbrif_mgr->PortStatusHandler(vbrif_ckv, ctrlr_name, domain_name,
                                         logical_port_id, oper_status,
                                         dbcm_, &cfg_lock_);
      DELETE_IF_NOT_NULL(vbrif_ckv);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Failed to update PortStatus. ctrlr_name=%s,"
                     "logical_port_id=%s, oper_status=%d urc=%d",
                     ctrlr_name, logical_port_id, oper_status, urc);
        return;
      }
      if (toupper(logical_port_id[0]) == 'T'
            && toupper(logical_port_id[1]) == 'P') {
        ConfigKeyVal *conv_vbrif_ckv = NULL;
        urc = vbrif_mgr->GetChildConvertConfigKey(conv_vbrif_ckv, NULL);
        if (urc != UPLL_RC_SUCCESS) {
          return;
        }
        urc = vbrif_mgr->PortStatusHandler(conv_vbrif_ckv, ctrlr_name,
                                           domain_name, logical_port_id,
                                           oper_status, dbcm_, &cfg_lock_);
        DELETE_IF_NOT_NULL(conv_vbrif_ckv);
        if (urc != UPLL_RC_SUCCESS) {
          UPLL_LOG_ERROR("Failed to update PortStatus. ctrlr_name=%s,"
                         "logical_port_id=%s, oper_status=%d urc=%d",
                         ctrlr_name, logical_port_id, oper_status, urc);
          return;
        }
      }
    }
  }

  if (toupper(logical_port_id[0]) == 'M'
        && toupper(logical_port_id[1]) == 'G') {
    uuk::VbrPortMapMoMgr *vbrpm_mgr = reinterpret_cast<uuk::VbrPortMapMoMgr *>
      (GetMoManager(UNC_KT_VBR_PORTMAP));
    if (vbrpm_mgr != NULL) {
      ConfigKeyVal *conv_vbrpm_ckv = NULL;
      urc = vbrpm_mgr->GetChildConfigKey(conv_vbrpm_ckv, NULL);
      if (urc != UPLL_RC_SUCCESS) {
        return;
      }
      urc = vbrpm_mgr->PortStatusHandler(conv_vbrpm_ckv, ctrlr_name,
                                         domain_name, logical_port_id,
                                         oper_status, dbcm_, &cfg_lock_);
      DELETE_IF_NOT_NULL(conv_vbrpm_ckv);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Failed to update PortStatus. ctrlr_name=%s,"
                       "logical_port_id=%s, oper_status=%d urc=%d",
                       ctrlr_name, logical_port_id, oper_status, urc);
        return;
      }
    }
  }
  if ((toupper(logical_port_id[0]) == 'T' ||
       toupper(logical_port_id[0]) == 'P') &&
       toupper(logical_port_id[1]) == 'P') {
    uuk::VtermIfMoMgr *vtermif_mgr = reinterpret_cast<uuk::VtermIfMoMgr *>
      (GetMoManager(UNC_KT_VTERM_IF));
    if (vtermif_mgr != NULL) {
      ConfigKeyVal *vtermif_ckv = NULL;
      urc = vtermif_mgr->GetChildConfigKey(vtermif_ckv, NULL);
      if (urc != UPLL_RC_SUCCESS) {
        return;
      }
      urc = vtermif_mgr->PortStatusHandler(vtermif_ckv, ctrlr_name,
                                           domain_name, logical_port_id,
                                           oper_status, dbcm_, &cfg_lock_);
      DELETE_IF_NOT_NULL(vtermif_ckv);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Failed to update PortStatus. ctrlr_name=%s,"
                     "logical_port_id=%s, oper_status=%d urc=%d",
                     ctrlr_name, logical_port_id, oper_status, urc);
        return;
      }
    }
  }
}

// NOTE: The functions invoked by this function, access GetAlarmConn(), and
// also take ConfigLock::CFG_READ_LOCK on UPLL_DT_RUNNING
void UpllConfigMgr::OnBoundaryStatusChange(const char *boundary_id,
                                           bool oper_status) {
  UPLL_FUNC_TRACE;
#if 1
  uuk::VlinkMoMgr *mgr = reinterpret_cast<uuk::VlinkMoMgr *>
      (GetMoManager(UNC_KT_VLINK));
  if (mgr != NULL) {
    upll_rc_t urc;
    urc = mgr->BoundaryStatusHandler(
        reinterpret_cast<uint8_t*>(const_cast<char*>(boundary_id)),
        oper_status, dbcm_, &cfg_lock_);
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Failed to Update BoundaryStatus. "
                     "boundary_id=%s oper_status=%s urc=%d",
                     boundary_id, (oper_status ? "UP" : "DOWN"), urc);
    }
  }
#else
  UPLL_LOG_DEBUG("Recevied boundary %s oper status %d\n",
                 boundary_id, oper_status);
#endif
}

void UpllConfigMgr::OnPolicerFullAlarm(
    string ctrlr_name, string domain_id, const key_vtn_t &key_vtn,
    const pfcdrv_policier_alarm_data_t &alarm_data, bool alarm_raised) {
  UPLL_FUNC_TRACE;
  namespace uuk = unc::upll::kt_momgr;
  ScopedConfigLock lock(cfg_lock_, kNormalTaskPriority,
                        UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  uuk::VbrPolicingMapMoMgr *mgr  = reinterpret_cast<uuk::VbrPolicingMapMoMgr*>
      (GetMoManager(UNC_KT_VBR_POLICINGMAP));
  if (mgr != NULL) {
    upll_rc_t urc;
    DalOdbcMgr *dom = NULL;
    if (UPLL_RC_SUCCESS != (urc = dbcm_->AcquireRoConn(&dom))) {
      return;
    }
    urc = mgr->OnPolicerFullAlarm(ctrlr_name, domain_id, key_vtn,
                                  alarm_data, alarm_raised, dom);
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Failed to handle PolicerFullAlarm. "
                     "ctrlr_name=%s, domain_id=%s, alarm_raised=%s urc=%d",
                     ctrlr_name.c_str(), domain_id.c_str(),
                     (alarm_raised ? "raised" : "recovered"), urc);
    }
    dbcm_->DalTxClose(dom, false);
    dbcm_->ReleaseRoConn(dom);
  }
}

void UpllConfigMgr::OnPolicerFailAlarm(
    string ctrlr_name, string domain_id, const key_vtn_t &key_vtn,
    const pfcdrv_policier_alarm_data_t &alarm_data, bool alarm_raised) {
  UPLL_FUNC_TRACE;
  namespace uuk = unc::upll::kt_momgr;
  ScopedConfigLock lock(cfg_lock_, kNormalTaskPriority,
                        UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  uuk::VbrPolicingMapMoMgr *mgr  = reinterpret_cast<uuk::VbrPolicingMapMoMgr*>
      (GetMoManager(UNC_KT_VBR_POLICINGMAP));

  if (mgr != NULL) {
    upll_rc_t urc;
    DalOdbcMgr *dom = NULL;
    if (UPLL_RC_SUCCESS != (urc = dbcm_->AcquireRoConn(&dom))) {
      return;
    }
    urc = mgr->OnPolicerFailAlarm(ctrlr_name, domain_id, key_vtn,
                                  alarm_data, alarm_raised, dom);
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Failed to handle PolicerFailAlarm. "
                     "ctrlr_name=%s, domain_id=%s, alarm_raised=%s urc=%d",
                     ctrlr_name.c_str(), domain_id.c_str(),
                     (alarm_raised ? "raised" : "recovered"), urc);
    }
    dbcm_->DalTxClose(dom, false);
    dbcm_->ReleaseRoConn(dom);
  }
}

void UpllConfigMgr::OnNwmonFaultAlarm(
    string ctrlr_name, string domain_id, const key_vtn_t &vtn_key,
    const pfcdrv_network_mon_alarm_data_t &alarm_data, bool alarm_raised) {
  UPLL_FUNC_TRACE;
  ScopedConfigLock lock(cfg_lock_, kNormalTaskPriority,
                        UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);
  uuk::NwMonitorMoMgr *nwm_mgr = reinterpret_cast<uuk::NwMonitorMoMgr*>
         (const_cast<MoManager*>(GetMoManager(UNC_KT_VBR_NWMONITOR)));
  if (nwm_mgr != NULL) {
    upll_rc_t urc;
    DalOdbcMgr *dom = dbcm_->GetAlarmRwConn();
    if (dom == NULL) { return; }
    urc = nwm_mgr->OnNwmonFault(ctrlr_name, domain_id, vtn_key, alarm_data,
                            alarm_raised, dom);
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Failed to handle NwmonFault. "
                     "ctrlr_name=%s, domain_id=%s, alarm_raised=%s urc=%d",
                     ctrlr_name.c_str(), domain_id.c_str(),
                     (alarm_raised? "raised" : "recovered"), urc);
    }
    dbcm_->DalTxClose(dom, (urc == UPLL_RC_SUCCESS));
    dbcm_->ReleaseRwConn(dom);
  }
}

/**
 * @brief : This function sends operational status alarm for a vnode if vif_name
 * is NULL, otherwise the alarm is generated for virtual interface.
 */
bool UpllConfigMgr::SendOperStatusAlarm(const char *vtn_name,
                                        const char *vnode_name,
                                        const char *vif_name,
                                        bool assert_alarm) {
  UPLL_FUNC_TRACE;
  std::string message;
  std::string message_summary;
  pfc::alarm::alarm_info_with_key_t data;

  if (vtn_name == NULL || vnode_name == NULL) {
    UPLL_LOG_DEBUG("Invalid argument vtn_name %p, vnode_name %p",
                   vtn_name, vnode_name);
    return false;
  }

  if (assert_alarm) {
    message = "Operational status down - ";
    message_summary = "Operational status down";
    data.alarm_class = pfc::alarm::ALM_WARNING;
    data.alarm_kind = 1;   // assert alarm
  } else {
    message = "Operational status up - ";
    message_summary = "Operational status up";
    data.alarm_class = pfc::alarm::ALM_NOTICE;
    data.alarm_kind = 0;   // clear alarm
  }

  // Add source of the alarm to message
  message += vnode_name;
  if (vif_name != NULL) {
    message +=  std::string(":") + vif_name;
  }

  data.apl_No = UNCCID_LOGICAL;
  std::string alarm_key(vnode_name);
  if (vif_name != NULL) {
    alarm_key.append(1, ':').append(vif_name);
  }
  data.alarm_key = const_cast<uint8_t *>(
      reinterpret_cast<const uint8_t *>(alarm_key.c_str()));
  data.alarm_key_size = alarm_key.length();

  data.alarm_category = UPLL_OPERSTATUS_ALARM;

  pfc::alarm::alarm_return_code_t ret = pfc::alarm::pfc_alarm_send_with_key(
      vtn_name, message, message_summary, &data, alarm_fd);
  if (ret != pfc::alarm::ALM_OK) {
    UPLL_LOG_WARN("Failed to %s invalid configuration alarm, Err=%d",
                  ((assert_alarm) ? "assert" : "clear"), ret);
    // return false;
    return true;
  }

  return true;
}

void UpllConfigMgr::GetBatchParamsFrmConfFile() {
  UPLL_FUNC_TRACE;

  pfc::core::ModuleConfBlock batch_conf_block(batch_config_mode_conf_blk);
  if (batch_conf_block.getBlock() != PFC_CFBLK_INVALID) {
    UPLL_LOG_TRACE("Block handle is valid");
    batch_timeout_ = batch_conf_block.getUint32("batch_timeout",
                                                default_batch_timeout);
    UPLL_LOG_DEBUG("BATCH default timeout from conf file %d", batch_timeout_);

    batch_commit_limit_ = batch_conf_block.getUint32(
        "batch_commit_limit", default_batch_commit_limit);
    UPLL_LOG_DEBUG("BATCH default commit limit value  from conf file %d",
                  batch_commit_limit_);
  }
  UPLL_LOG_INFO("BATCH timeout value %d", batch_timeout_);
  UPLL_LOG_INFO("BATCH commit limit value %d", batch_commit_limit_);
}

upll_rc_t UpllConfigMgr::ValidateBatchConfigMode(uint32_t session_id,
                                                 uint32_t config_id) {
#if 0
  upll_rc_t urc = ValidSession(session_id, config_id, TC_CONFIG_GLOBAL, NULL);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Invalid session_id %u and/or config_id %u in the IPC request"
                  ". Err: %d", session_id, config_id, urc);
    return urc;
  }
#endif
  TcConfigMode cfg_mode = TC_CONFIG_INVALID;
  std::string vtn_name;
  upll_rc_t urc = GetConfigMode(session_id, config_id, &cfg_mode, &vtn_name);
  if (urc == UPLL_RC_SUCCESS) {
// warn: The locking code is untidy. It could have been made simpler. -- Ravi
    batch_mutex_lock_.lock();
    if (batch_mode_in_progress_) {
      if (((cfg_mode == TC_CONFIG_GLOBAL || cfg_mode == TC_CONFIG_VIRTUAL) &&
           (cfg_mode == cur_batch_cfg_mode_)) ||
          ((cfg_mode == TC_CONFIG_VTN) && (cfg_mode == cur_batch_cfg_mode_) &&
           (vtn_name.compare(cur_batch_cfg_mode_vtn_name_) == 0))) {
          urc = UPLL_RC_SUCCESS;
        } else {
          UPLL_LOG_INFO("Batch is already in progress in mode <%u,%s>,"
                        "Request from <%u,%u>",
                        cur_batch_cfg_mode_,
                        cur_batch_cfg_mode_vtn_name_.c_str(),
                        session_id, config_id);
           batch_mutex_lock_.unlock();
          return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
        }
    }
  } else if (urc == UPLL_RC_ERR_BAD_CONFIG_OR_SESSION_ID) {
    UPLL_LOG_INFO("Validation failed. Urc=%d", urc);
    // batch_mutex_lock_.unlock();
    return urc;
  } else {
    UPLL_LOG_INFO("Validation failed. Urc=%d", urc);
    // batch_mutex_lock_.unlock();
    return urc;
  }
  batch_mutex_lock_.unlock();
  return urc;
}

upll_rc_t UpllConfigMgr::RegisterBatchTimeoutCB(uint32_t session_id,
                                                uint32_t config_id) {
  pfc_timespec_t  timeout;

  timeout.tv_sec = batch_timeout_;
  timeout.tv_nsec = 0;

  int err = batch_timer_->post(&timeout, batch_timer_func_, &batch_timeout_id_);
  if (err != 0) {
    UPLL_LOG_ERROR("BATCH Timer post() operation is failed. Err=%d", err);
    return UPLL_RC_ERR_GENERIC;
  }
  UPLL_LOG_TRACE("BATCH Timer %d is registered.", batch_timeout_id_);
  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllConfigMgr::OnBatchStart(uint32_t session_id, uint32_t config_id) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  upll_rc_t urc = ValidateBatchConfigMode(session_id, config_id);
  if (urc != UPLL_RC_SUCCESS) {
    return urc;
  }
  TcConfigMode cfg_mode;
  std::string cfg_mode_vtn_name;
  bool batch_reentered = false;
  batch_mutex_lock_.lock();
  batch_reentered = batch_mode_in_progress_;
  if (!batch_mode_in_progress_) {
    UPLL_LOG_INFO("Entering BATCH mode");
    urc = GetConfigMode(session_id, config_id, &cfg_mode, &cfg_mode_vtn_name);
     if (UPLL_RC_SUCCESS != urc) {
      batch_mutex_lock_.unlock();
      return urc;
    }
    urc = RegisterBatchTimeoutCB(session_id, config_id);
    if (UPLL_RC_SUCCESS != urc) {
      batch_mutex_lock_.unlock();
      return urc;
    }
    batch_mode_in_progress_ = true;
    batch_op_cnt_ = 0;
    cur_batch_cfg_mode_ = cfg_mode;
    cur_batch_cfg_mode_vtn_name_ = cfg_mode_vtn_name;
  }
  batch_mutex_lock_.unlock();

  if (batch_reentered) {
    UPLL_LOG_DEBUG("BATCH mode is already in progress, performing DB Commit");
    urc = DbCommitIfInBatchMode(__FUNCTION__, kNormalTaskPriority);
  }

  return urc;
}

upll_rc_t UpllConfigMgr::OnBatchAlive(uint32_t session_id, uint32_t config_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = ValidateBatchConfigMode(session_id, config_id);
  if (urc != UPLL_RC_SUCCESS) {
    return urc;
  }

  batch_mutex_lock_.lock();
  if (batch_mode_in_progress_) {
    if (batch_timer_ != NULL) {
      batch_timer_->cancel(batch_timeout_id_);
      UPLL_LOG_TRACE("BATCH Timer is canceled for id %d", batch_timeout_id_);
      batch_timeout_id_ = PFC_TIMER_INVALID_ID;
      urc = RegisterBatchTimeoutCB(session_id, config_id);
      if (UPLL_RC_SUCCESS != urc) {
        batch_mutex_lock_.unlock();
        return urc;
      }
    }
  } else {
    // Untimely request for BATCH-ALIVE
    UPLL_LOG_INFO("Ignoring BATCH-ALIVE because BATCH mode is not active");
    batch_mutex_lock_.unlock();
    return UPLL_RC_SUCCESS;
  }
  batch_mutex_lock_.unlock();

  return UPLL_RC_SUCCESS;
}

// @param timedout will be false on user request, true from BATCH timeout.
upll_rc_t UpllConfigMgr::OnBatchEnd(uint32_t session_id, uint32_t config_id,
                                    bool timedout) {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s,%d,%d,%d ***", __FUNCTION__,
                session_id, config_id, timedout);

  ScopedConfigLock scfg_lock(cfg_lock_, kNormalTaskPriority,
                              UPLL_DT_CANDIDATE, ConfigLock::CFG_WRITE_LOCK,
                              UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK,
                              UPLL_DT_IMPORT, ConfigLock::CFG_READ_LOCK,
                              // UPLL_DT_AUDIT, ConfigLock::CFG_READ_LOCK,
                              UPLL_DT_STARTUP, ConfigLock::CFG_READ_LOCK);
//  pfc::core::ScopedMutex batch_mutex(batch_mutex_lock_);

  upll_rc_t urc = UPLL_RC_SUCCESS;
  if (!timedout) {
    urc = ValidateBatchConfigMode(session_id, config_id);
    if (urc != UPLL_RC_SUCCESS) {
      return urc;
    }
  } else if ((urc = ContinueActiveProcess()) != UPLL_RC_SUCCESS) {
    return urc;
  }

  if (batch_mode_in_progress_) {
    UPLL_LOG_INFO("Exiting from BATCH mode. Performing DB Commit");
    batch_mode_in_progress_ = false;
    if (!timedout) {
      batch_timer_->cancel(batch_timeout_id_);
    }
    batch_timeout_id_ = PFC_TIMER_INVALID_ID;
  } else {
    UPLL_LOG_INFO("Not in BATCH mode. Performing DB Commit");
  }
  cur_batch_cfg_mode_ = TC_CONFIG_INVALID;
  cur_batch_cfg_mode_vtn_name_.clear();

  DalOdbcMgr *dom = dbcm_->GetConfigRwConn();
  if (dom == NULL) { return UPLL_RC_ERR_GENERIC; }
  urc = dbcm_->DalTxClose(dom, true);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("DB Commit Err:%d in BATCH-END", urc);
  }
  dbcm_->ReleaseRwConn(dom);

  return urc;
}

void UpllConfigMgr::TerminateBatch() {
  pfc::core::ScopedMutex mutex(batch_mutex_lock_);
  batch_mode_in_progress_ = false;
  if (batch_timeout_id_ != PFC_TIMER_INVALID_ID) {
    batch_timer_->cancel(batch_timeout_id_);
  }
  batch_timeout_id_ = PFC_TIMER_INVALID_ID;
}


/**
 * @brief : Sends path fault alarm. "*" can be passed for ctrlr, domain, vtn
 */
bool UpllConfigMgr::SendPathFaultAlarm(const char *ctr_na,
                                       const char *domain_na,
                                       const char *vtn_na,
                                       upll_alarm_kind_t alarm_kind) {
  UPLL_FUNC_TRACE;
  std::string message;
  std::string message_summary;
  pfc::alarm::alarm_info_with_key_t data;

  if (vtn_na == NULL || ctr_na == NULL || domain_na == NULL) {
    UPLL_LOG_ERROR("Invalid argument vtn_name %p, ctr_name %p domain_name %p",
                   vtn_na, ctr_na, domain_na);
    return false;
  }
  std::string vtn_name(vtn_na);
  std::string ctr_name(ctr_na);
  std::string domain_name(strcmp(uuk::kDefaultDomainId, domain_na) ?
                          domain_na : "default-domain");

  if (alarm_kind == UPLL_ASSERT_WITH_TRAP) {
    message = "Path fault - ";
    message_summary = "Path fault";
    data.alarm_class = pfc::alarm::ALM_WARNING;
  } else if (alarm_kind == UPLL_CLEAR_WITH_TRAP ||
             alarm_kind == UPLL_CLEAR_WITHOUT_TRAP) {
    message = "No path fault - ";
    message_summary = "No path fault";
    data.alarm_class = pfc::alarm::ALM_NOTICE;
  } else {
    UPLL_LOG_DEBUG("Invalid alarm kind %d, vtn %s, ctr %s domain %s",
                   alarm_kind, vtn_na, ctr_na, domain_na);
    return false;
  }

  // Alarm kind
  data.alarm_kind = alarm_kind;
  // Alarm key
  std::string alarm_key = vtn_name + ":" + ctr_name + ":" + domain_name;

  // Add source of the alarm to message
  message += alarm_key;

  data.apl_No = UNCCID_LOGICAL;

  data.alarm_key = const_cast<uint8_t *>(
      reinterpret_cast<const uint8_t *>(alarm_key.c_str()));
  data.alarm_key_size = alarm_key.length();

  data.alarm_category = UPLL_PATHFAULT_ALARM;

  pfc::alarm::alarm_return_code_t ret =  pfc::alarm::ALM_OK;
  int retry_count = 0;
  while ((ret = pfc::alarm::pfc_alarm_send_with_key(
      vtn_name, message, message_summary, &data, alarm_fd)) ==
      pfc::alarm::ALM_EAGAIN && retry_count < 10) {
    UPLL_LOG_INFO("Alarm failed for vtn : %s. Retrying.....", vtn_name.c_str());
    usleep(10*1000);
    ++retry_count;
  }

  if (ret != pfc::alarm::ALM_OK) {
    UPLL_LOG_WARN("Failed to %s path fault alarm, Err=%d",
                  ((alarm_kind == UPLL_ASSERT_WITH_TRAP) ? "assert" : "clear"),
                  ret);
    // return false;
    return true;
  }
  UPLL_LOG_DEBUG("msg:%s alarm_kind: %d", message.c_str(), data.alarm_kind);
  return true;
}

bool UpllConfigMgr::SendSpineThresholdAlarm(
    const char *unw_name, const char *spine_id, bool assert_alarm) {
  UPLL_FUNC_TRACE;
  std::string message;
  const string vtn_name = "";

  std::string message_summary;
  pfc::alarm::alarm_info_with_key_t data;
  if (unw_name == NULL || spine_id == NULL) {
    UPLL_LOG_ERROR("Invalid argument, unified network name %p, spine id %p",
                   unw_name, spine_id);
    return false;
  }
  std::string unified_nw_name(unw_name);
  std::string spine_name(spine_id);
  if (assert_alarm) {
    message = "Spine label assignment exceeds threshold for ";
    message_summary = "Spine label exceeded alarm";
    data.alarm_class = pfc::alarm::ALM_WARNING;
    data.alarm_kind = 1;
  } else {
    message = "Spine label assignment is not exceeding threshold for ";
    message_summary = "No spine label exceeded alarm";
    data.alarm_class = pfc::alarm::ALM_NOTICE;
    data.alarm_kind = 0;
  }
  // Alarm key
  std::string alarm_key = unified_nw_name + ":" + spine_name;
  // Add source of the alarm to message
  message += alarm_key;
  data.apl_No = UNCCID_LOGICAL;
  data.alarm_key = const_cast<uint8_t *>(
      reinterpret_cast<const uint8_t *>(alarm_key.c_str()));
  data.alarm_key_size = alarm_key.length();

  data.alarm_category = UPLL_SPINE_THRESHOLD_ALARM;
  pfc::alarm::alarm_return_code_t ret =  pfc::alarm::ALM_OK;
  // Retry 10 times if alaram sending fails
  int retry_count = 0;
  while ((ret = pfc::alarm::pfc_alarm_send_with_key(
              vtn_name, message, message_summary, &data, alarm_fd)) ==
      pfc::alarm::ALM_EAGAIN && retry_count < 10) {
    UPLL_LOG_DEBUG("Threshold alarm (%d) for %s failed", assert_alarm,
                   alarm_key.c_str());
    usleep(10*1000);
    ++retry_count;
  }
  if (ret != pfc::alarm::ALM_OK) {
    UPLL_LOG_WARN("Failed to %s Spine label exceeded alarm, Err=%d",
                  ((assert_alarm) ? "assert" : "clear"), ret);
     return false;
  }

  // Update alarm cache
  sd_threshold_alarm_lock_.lock();
  std::pair<std::string, std::string> unw_sd_pair =
      std::make_pair(unw_name, spine_id);
  if (assert_alarm) {
      sd_threshold_alarm_set_.insert(unw_sd_pair);
  } else {
    std::set<std::pair<std::string, std::string> >::iterator iter =
        sd_threshold_alarm_set_.find(unw_sd_pair);
    if (iter != sd_threshold_alarm_set_.end()) {
      sd_threshold_alarm_set_.erase(iter);
    }
  }
  sd_threshold_alarm_lock_.unlock();

  UPLL_LOG_DEBUG("msg:%s alarm_kind: %d", message.c_str(), assert_alarm);
  return true;
}

void UpllConfigMgr::OnVtnIDExhaustionAlarm(
                    const char *ctrlr_name, const char *domain_name,
                    key_vtn vtn_key, bool alarm_asserted) {
  UPLL_FUNC_TRACE;
  pfc_sem_wait(&sem_alarm_commit_syc_);  // wait for commit or audit to finish
  pfc_sem_post(&sem_alarm_commit_syc_);

  if (!ctrlr_name || !domain_name) {
    UPLL_LOG_ERROR("ctrlr, domain is NULL");
    return;
  }
  uuk::VtnMoMgr *mgr = reinterpret_cast<uuk::VtnMoMgr *>
      (GetMoManager(UNC_KT_VTN));
  if (mgr != NULL) {
    upll_rc_t urc = UPLL_RC_SUCCESS;
    urc = mgr->VtnExhaustionHandler(
        reinterpret_cast<uint8_t*>(const_cast<char*>(ctrlr_name)),
        reinterpret_cast<uint8_t*>(const_cast<char*>(domain_name)),
        vtn_key, alarm_asserted, dbcm_, &cfg_lock_);
    if (urc != UPLL_RC_SUCCESS)
      UPLL_LOG_ERROR("Failed to handle VtnIDExhaustionAlarm. "
                 "ctrlr_name=%s, alarm %s, Urc=%d", ctrlr_name,
                 (alarm_asserted ? "asserted" : "recovered"), urc);
  }
}
void UpllConfigMgr::OnPathFaultAlarm(const char *ctrlr_name,
                                     const char *domain_name,
                                     bool alarm_asserted) {
  UPLL_FUNC_TRACE;
  pfc_sem_wait(&sem_alarm_commit_syc_);  // wait for commit or audit to finish
  pfc_sem_post(&sem_alarm_commit_syc_);
  CtrlrMgr *ctr_mgr = CtrlrMgr::GetInstance();
  bool result(false);
  if (ctrlr_name == NULL || domain_name == NULL) {
    UPLL_LOG_ERROR("ctrlr_name or domain_name is NULL");
    return;
  }
  string dom_id(domain_name);
  if (dom_id != "*") {
    result = ctr_mgr->UpdatePathFault
        (ctrlr_name, domain_name, alarm_asserted);
  } else {
    // path fault clear on user audit;
    UPLL_LOG_DEBUG("Clear path fault on user initiated audit for ctr_name %s",
                   ctrlr_name);
    result = true;
  }

  if (!result) {
    // no change in OperStatus
    UPLL_LOG_DEBUG("No change in OperStatus ctr_name %s domain_name %s",
                   ctrlr_name, domain_name);
    return;
  }
  uuk::VtnMoMgr *mgr = reinterpret_cast<uuk::VtnMoMgr *>
      (GetMoManager(UNC_KT_VTN));
  if (mgr != NULL) {
    UPLL_LOG_INFO("Controller(%s), domain(%s) pathfault start : alarm(%d)",
                   ctrlr_name, domain_name, alarm_asserted);
    set<string> dom_list;
    if (dom_id == "*") {
      ctr_mgr->GetPathFaultDomains(ctrlr_name, &dom_list);
    } else {
      dom_list.insert(domain_name);
    }
    set<string>::iterator dom_it = dom_list.begin(), end_it = dom_list.end();
    for (; dom_it != end_it; ++dom_it) {
      upll_rc_t urc;
      urc = mgr->PathFaultHandler(
        reinterpret_cast<uint8_t*>(const_cast<char*>(ctrlr_name)),
        reinterpret_cast<uint8_t*>(const_cast<char*>((*dom_it).c_str())),
        alarm_asserted, dbcm_, &cfg_lock_);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_ERROR("Failed to handle PathFaultAlarm. "
                     "ctrlr_name=%s, alarm %s, Urc=%d", ctrlr_name,
                     (alarm_asserted ? "asserted" : "recovered"), urc);
        dom_list.clear();
        return;
      }
    }
    dom_list.clear();
    UPLL_LOG_INFO("Controller(%s), domain(%s) pathfault end: alarm(%d)",
                   ctrlr_name, domain_name, alarm_asserted);
  }
}

upll_rc_t UpllConfigMgr::PopulateCtrlrInfo() {
  UPLL_FUNC_TRACE;
  UPLL_LOG_INFO("*** %s ***", __FUNCTION__);
  uuk::VtnMoMgr *mgr = reinterpret_cast<uuk::VtnMoMgr *>
      (GetMoManager(UNC_KT_VTN));
  upll_rc_t urc(UPLL_RC_SUCCESS);
  if (mgr != NULL) {
    unc::upll::dal::DalBindInfo dal_bind_info(1);
    ConfigKeyVal *ck_vtn = NULL;
    urc = mgr->GetChildConfigKey(ck_vtn, NULL);
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("GetChildConfigKey failed");
      return UPLL_RC_ERR_GENERIC;
    }
    mgr->BindInfoBasedonOperation(&dal_bind_info, ck_vtn, uuk::UNC_START_UP);
    urc = mgr->PopulateCtrlrInfo(&dal_bind_info, dbcm_, &cfg_lock_,
                                 ck_vtn, uuk::UNC_START_UP);
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_ERROR("Failed to Populate info. Urc=%d", urc);
    }
    DELETE_IF_NOT_NULL(ck_vtn);
  }
  return urc;
}

void UpllConfigMgr::ClearAllAlarms() {
  UPLL_FUNC_TRACE;
  CtrlrMgr *ctr_mgr = CtrlrMgr::GetInstance();
  // clear all path fault alarm if already asserted
  if (ctr_mgr->IsPathFaultOccured("*", "*")) {
    SendPathFaultAlarm("*", "*", "*", UPLL_CLEAR_WITH_TRAP);
    ctr_mgr->ClearPathfault("*", "*");
  }
  ctr_mgr->DeleteFromUnknownCtrlrList("*");
  ctr_mgr->RemoveCtrlrFromIgnoreList("*");
  ctr_mgr->ClearVtnExhaustionMap();

  // clear all invalid config Alarms
  std::list<string> invalid_config_ctrlr;
  ctr_mgr->GetInvalidConfigList(invalid_config_ctrlr);
  for (std::list<std::string>::iterator it = invalid_config_ctrlr.begin();
      it != invalid_config_ctrlr.end(); ++it) {
    SendInvalidConfigAlarm(it->c_str(), false);
    ctr_mgr->UpdateInvalidConfig(it->c_str(), false);
  }
  // Clear SpineThreshold alarm if already asserted
  sd_threshold_alarm_lock_.lock();   // LOCK
  if (sd_threshold_alarm_set_.empty()) {
      sd_threshold_alarm_lock_.unlock();   // UNLOCK
  } else {
      std::set<std::pair<std::string, std::string> > threshold_clone_set;
      threshold_clone_set.insert(sd_threshold_alarm_set_.begin(),
                                 sd_threshold_alarm_set_.end());
      sd_threshold_alarm_lock_.unlock();   // UNLOCK
      for (std::set<std::pair<std::string, std::string> >::iterator iter
           = threshold_clone_set.begin();
           iter != threshold_clone_set.end(); iter++) {
        SendSpineThresholdAlarm(((*iter).first).c_str(),
                                ((*iter).second).c_str(), false);
      }
      threshold_clone_set.clear();
    }
}

void UpllConfigMgr::ClearPathFaultAlarms(const char *ctrlr_name) {
  UPLL_FUNC_TRACE;
  CtrlrMgr *ctr_mgr = CtrlrMgr::GetInstance();
  OnControllerStatusChange(ctrlr_name, UPPL_CONTROLLER_OPER_DOWN);
  // clear all path fault alarm if already asserted
  if (!ctr_mgr->IsPathFaultOccured(ctrlr_name, "*")) {
    // No path fault already asserted;
    return;
  }
  SendPathFaultAlarm(ctrlr_name, "*", "*", UPLL_CLEAR_WITH_TRAP);
}

upll_rc_t UpllConfigMgr::PurgeCandidate(DalDmlIntf *dmi) {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = UPLL_RC_SUCCESS;
  unc_key_type_t kt;

  if (dmi == NULL) { return UPLL_RC_ERR_GENERIC; }

  const std::list<unc_key_type_t> *post_list =
      cktt_.get_reverse_postorder_list();
  for (std::list<unc_key_type_t>::const_iterator post_it = post_list->begin();
       post_it != post_list->end(); post_it++) {
    kt = *post_it;
    std::map<unc_key_type_t, MoManager*>::iterator momgr_it =
      upll_kt_momgrs_.find(kt);
    if (momgr_it != upll_kt_momgrs_.end()) {
      MoManager *momgr = momgr_it->second;
      urc = momgr->PurgeCandidate(kt, import_ctrlr_id_.c_str(), dmi);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_INFO("PurgeCandidate failed %d", urc);
        break;
      }
    }
  }
  return urc;
}

upll_rc_t UpllConfigMgr::GetControllerSatusFromPhysical(uint8_t *ctrlr_id,
                                                        bool &is_audit_wait) {
  UPLL_FUNC_TRACE;
  IpcResponse ipc_resp;
  ConfigKeyVal *ctrlr_ckv = NULL;
  upll_rc_t result_code = UPLL_RC_SUCCESS;

  key_ctr *ctrlr  = static_cast<key_ctr *>
                        (ConfigKeyVal::Malloc(sizeof(key_ctr)));
  uuu::upll_strncpy(ctrlr->controller_name,
                    reinterpret_cast<const char *>(ctrlr_id),
                    (uuk::kMaxLenCtrlrId+1));
  ctrlr_ckv = new ConfigKeyVal(UNC_KT_CONTROLLER,
                              IpctSt::kIpcStKeyCtr, ctrlr, NULL);

  memset(&ipc_resp, 0, sizeof(IpcResponse));
  IpcRequest ipc_req;
  memset(&ipc_req, 0, sizeof(ipc_req));
  ipc_req.header.clnt_sess_id = USESS_ID_UPLL;
  ipc_req.header.operation = UNC_OP_READ;
  ipc_req.header.datatype = UPLL_DT_STATE;
  ipc_req.ckv_data = ctrlr_ckv;
  ipc_req.header.rep_count = 1000;

  if (!IpcUtil::SendReqToPhysical(UPPL_IPC_SVC_NAME, UPPL_SVC_READREQ,
                                  &ipc_req, &ipc_resp)) {
    UPLL_LOG_INFO("Send Request to Physical failed");
  }
  result_code = ipc_resp.header.result_code;
  if ((result_code != UPLL_RC_SUCCESS)) {
    delete ctrlr_ckv;
    ctrlr_ckv = NULL;
    is_audit_wait = false;
    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    return result_code;
  }
  if (!ipc_resp.ckv_data) {
    UPLL_LOG_INFO(" Request to Physical was failed");
    delete ctrlr_ckv;
    ctrlr_ckv = NULL;
    is_audit_wait = false;
    // Become double delete
//    DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
    return UPLL_RC_ERR_GENERIC;
  }
  ctrlr_ckv->ResetWith(ipc_resp.ckv_data);
  DELETE_IF_NOT_NULL(ipc_resp.ckv_data);
  val_ctr_st *ctrlr_val = static_cast<val_ctr_st *>
                                      (GetVal(ctrlr_ckv));
  if (!ctrlr_val || (ctrlr_val->valid[kIdxOperStatus] !=
                    UNC_VF_VALID)) {
    UPLL_LOG_DEBUG("Returning error \n");
    DELETE_IF_NOT_NULL(ctrlr_ckv);
    is_audit_wait = false;
    return UPLL_RC_ERR_GENERIC;
  }
  switch (ctrlr_val->oper_status) {
    case UPPL_CONTROLLER_OPER_WAITING_AUDIT: {
      UPLL_LOG_TRACE("Controller status is Audit Waiting");
      is_audit_wait = true;
      break;
    }
    default: {
      UPLL_LOG_TRACE("Controller status is Non Audit Waiting %d",
                     ctrlr_val->oper_status);
      is_audit_wait = false;
      break;
    }
  }
  if (ctrlr_ckv)
    delete ctrlr_ckv;
  return result_code;
}

uint32_t UpllConfigMgr::GetTxUpdateTaskqParamsFrmConfFile() {
  UPLL_FUNC_TRACE;

  pfc::core::ModuleConfBlock taskq_conf_block(batch_config_mode_conf_blk);
  uint32_t no_of_tx_taskqs = 0;
  if (taskq_conf_block.getBlock() != PFC_CFBLK_INVALID) {
    UPLL_LOG_TRACE("Block handle is valid");
    no_of_tx_taskqs = taskq_conf_block.getUint32("max_task_queues",
                                                  default_tx_update_taskqs);
    UPLL_LOG_DEBUG("Txupdate default Taskqs  from conf file %d",
                   no_of_tx_taskqs);
  }
  UPLL_LOG_INFO("TxUpdate taskqs are %d", no_of_tx_taskqs);
  return no_of_tx_taskqs;
}

upll_rc_t UpllConfigMgr::GetVtnName(const IpcReqRespHeader &msghdr,
                                    const ConfigKeyVal &ckv,
                                    char **vtn_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t result_code = UPLL_RC_SUCCESS;
  unc_key_type_t key_type = ckv.get_key_type();

  switch (key_type) {
    case UNC_KT_VTN_MAPPING_CONTROLLER: {
        key_vtn_controller *vtn_ctr_key =
               reinterpret_cast<key_vtn_controller *>(ckv.get_key());
        *vtn_id = strdup(reinterpret_cast<const char*>(
                       vtn_ctr_key->vtn_key.vtn_name));
        break;
    }
    case UNC_KT_VTN:
    case UNC_KT_VTN_POLICINGMAP: {
      key_vtn *vtn_key = static_cast<key_vtn_t *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>(vtn_key->vtn_name));
      break;
    }
    case UNC_KT_VBRIDGE:
    case UNC_KT_VROUTER:
    case UNC_KT_VUNKNOWN:
    case UNC_KT_VTEP:
    case UNC_KT_VTUNNEL:
    case UNC_KT_VTERMINAL:
    case UNC_KT_VBR_POLICINGMAP: {
      key_vbr *vbr_key = reinterpret_cast<key_vbr_t *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
               (vbr_key->vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VBR_PORTMAP: {
      key_vbr_portmap_t *vbrpm_key = static_cast<key_vbr_portmap_t*>
                                     (ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                      (vbrpm_key->vbr_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VBR_VLANMAP: {
      key_vlan_map_t *vlanmap_key = reinterpret_cast<key_vlan_map_t *>
                                   (ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                      (vlanmap_key->vbr_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VBR_IF:
    case UNC_KT_VTERM_IF:
    case UNC_KT_VTEP_IF:
    case UNC_KT_VRT_IF:
    case UNC_KT_VTUNNEL_IF:
    case UNC_KT_VUNK_IF:
    case UNC_KT_VBRIF_POLICINGMAP:
    case UNC_KT_VTERMIF_POLICINGMAP: {
      key_vbr_if_t *vbr_if_key = reinterpret_cast<key_vbr_if_t *>
                                (ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                      (vbr_if_key->vbr_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VRT_IPROUTE: {
      key_static_ip_route *iproute_key = reinterpret_cast<key_static_ip_route *>
                                         (ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                      (iproute_key->vrt_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_DHCPRELAY_SERVER: {
      key_dhcp_relay_server *dhcp_rs_key =
            reinterpret_cast<key_dhcp_relay_server *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                      (dhcp_rs_key->vrt_key.vtn_key.vtn_name));
       break;
    }
    case UNC_KT_DHCPRELAY_IF: {
      key_dhcp_relay_if *dhcp_if = reinterpret_cast<key_dhcp_relay_if *>
                                   (ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                      (dhcp_if->vrt_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VTEP_GRP: {
      key_vtep_grp *vtep_grp_key = reinterpret_cast<key_vtep_grp *>
                                  (ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                      (vtep_grp_key->vtn_key.vtn_name));
       break;
    }
    case UNC_KT_VTEP_GRP_MEMBER: {
      key_vtep_grp_member  *vtep_grp_mem_key =
         reinterpret_cast<key_vtep_grp_member_t *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
         (vtep_grp_mem_key->vtepgrp_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VLINK: {
      key_vlink *vlink_key = reinterpret_cast<key_vlink_t *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                      (vlink_key->vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VBR_NWMONITOR: {
      key_nwm_t *nwm = reinterpret_cast<key_nwm_t *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                      (nwm->vbr_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VBR_NWMONITOR_HOST: {
      key_nwm_host *host_key = reinterpret_cast<key_nwm_host *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                (host_key->nwm_key.vbr_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VTN_FLOWFILTER: {
      key_vtn_flowfilter *vtn_ff_key = reinterpret_cast<key_vtn_flowfilter_t*>
                                      (ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                      (vtn_ff_key->vtn_key.vtn_name));
       break;
    }
    case UNC_KT_VTN_FLOWFILTER_ENTRY: {
      key_vtn_flowfilter_entry_t *vtn_ffe_key = reinterpret_cast
         <key_vtn_flowfilter_entry_t *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
               (vtn_ffe_key->flowfilter_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VTN_FLOWFILTER_CONTROLLER: {
      key_vtn_flowfilter_controller_t *vtn_ff_ctr =
          reinterpret_cast<key_vtn_flowfilter_controller_t*>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                      (vtn_ff_ctr->vtn_key.vtn_name));
       break;
    }
    case UNC_KT_VBR_FLOWFILTER: {
      key_vbr_flowfilter_t *vbr_ff_key = reinterpret_cast
            <key_vbr_flowfilter_t *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
           (vbr_ff_key->vbr_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VBR_FLOWFILTER_ENTRY: {
      key_vbr_flowfilter_entry_t *vbr_ffe_key =
        reinterpret_cast<key_vbr_flowfilter_entry_t *> (ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
           (vbr_ffe_key->flowfilter_key.vbr_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VBRIF_FLOWFILTER:
    case UNC_KT_VRTIF_FLOWFILTER:
    case UNC_KT_VTERMIF_FLOWFILTER: {
      key_vbr_if_flowfilter_t *vbr_if_ff_key = reinterpret_cast
          <key_vbr_if_flowfilter_t *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
          (vbr_if_ff_key->if_key.vbr_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VBRIF_FLOWFILTER_ENTRY:
    case UNC_KT_VRTIF_FLOWFILTER_ENTRY:
    case UNC_KT_VTERMIF_FLOWFILTER_ENTRY: {
      key_vbr_if_flowfilter_entry_t *vbr_if_ffe_key = reinterpret_cast
         <key_vbr_if_flowfilter_entry_t *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
          (vbr_if_ffe_key->flowfilter_key.if_key.vbr_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VBR_POLICINGMAP_ENTRY: {
      key_vbr_policingmap_entry *vtn_pme_key =
         reinterpret_cast<key_vbr_policingmap_entry *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast <const char*>
                  (vtn_pme_key->vbr_key.vtn_key.vtn_name));
      break;
     }
    case UNC_KT_VBRIF_POLICINGMAP_ENTRY: {
      key_vbrif_policingmap_entry *vbr_if_pme =
          reinterpret_cast<key_vbrif_policingmap_entry *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                  (vbr_if_pme->vbrif_key.vbr_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VTN_DATAFLOW: {
      key_vtn_dataflow_t *vtn_df_key = reinterpret_cast<key_vtn_dataflow_t *>
                                  (ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                     (vtn_df_key->vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VTERMIF_POLICINGMAP_ENTRY: {
      key_vtermif_policingmap_entry *vterm_if_pme =
         reinterpret_cast<key_vtermif_policingmap_entry *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                 (vterm_if_pme->vterm_if_key.vterm_key.vtn_key.vtn_name));
      break;
    }
    case UNC_KT_VTN_UNIFIED: {
      key_vtn_unified *vtn_unified_key =
        reinterpret_cast<key_vtn_unified *>(ckv.get_key());
      *vtn_id = strdup(reinterpret_cast<const char*>
                 (vtn_unified_key->vtn_key.vtn_name));
      break;
    }
    default:
      UPLL_LOG_INFO("key type not belongs to VTN config mode and so"
                    "vtn name not exists");
      return UPLL_RC_ERR_GENERIC;
  }
  return result_code;
}

upll_rc_t UpllConfigMgr::HandleSpineThresholdAlarm(bool assert_alarm) {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = UPLL_RC_SUCCESS;
  uuk::UNWSpineDomainMoMgr *mgr  = reinterpret_cast<uuk::UNWSpineDomainMoMgr*>
      (GetMoManager(UNC_KT_UNW_SPINE_DOMAIN));
  if (mgr != NULL) {
    DalOdbcMgr *dom = dbcm_->GetAlarmRwConn();
    if (dom == NULL) {
      UPLL_LOG_ERROR("Failed to get the alarm connection");
      return UPLL_RC_ERR_GENERIC;
    }
    urc = mgr->HandleSpineThresholdAlarm(assert_alarm, dom);
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_INFO("Failed to process threshold alarms, err %d", urc);
    }
    urc = dbcm_->DalTxClose(dom, (urc == UPLL_RC_SUCCESS));
    dbcm_->ReleaseRwConn(dom);
  }
  return urc;
}
// NOLINT
}  // namespace config_momgr
}  // namespace upll
}  // namespace unc
