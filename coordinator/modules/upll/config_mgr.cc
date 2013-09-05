/*
 * Copyright (c) 2012-2013 NEC Corporation
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

#include "pfcxx/module.hh"

#include "unc/upll_svc.h"
#include "unc/upll_errno.h"

#include "unc/pfcdriver_include.h"

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

namespace unc {
namespace upll {
namespace config_momgr {

using unc::upll::ipc_util::IpctSt;
using unc::upll::ipc_util::IpcUtil;
using unc::upll::ipc_util::ConfigKeyVal;
using unc::upll::ipc_util::ConfigVal;
using unc::upll::ipc_util::IpcReqRespHeader;
using unc::upll::ipc_util::IpcRequest;
using unc::upll::ipc_util::IpcResponse;
using unc::upll::ipc_util::KtUtil;

using unc::upll::upll_util::upll_strncpy;

namespace uuk = unc::upll::kt_momgr;
namespace uudal = unc::upll::dal;

UpllConfigMgr *UpllConfigMgr::singleton_instance_;

/*
 * UpllConfigMgr::UpllConfigMgr
 * Constructor of UpllConfigMgr instance.
 */
UpllConfigMgr::UpllConfigMgr()
    : cktt_(UNC_KT_ROOT), iktt_(UNC_KT_ROOT), tclib_impl_(this) {
      UPLL_LOG_DEBUG("constructor");
      commit_in_progress_ = false;
      audit_in_progress_ = false;
      import_in_progress_ = false;
      shutting_down_ = false;
      node_active_ = false;
      candidate_dirty_qc_ = false;
      alarm_fd = -1;
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

  if (!BuildKeyTree())
    return false;

#ifdef PFC_VERBOSE_DEBUG
  DumpKeyTree();
#endif

  unc::upll::ipc_util::IpctSt::RegisterAll();

  if (!CreateMoManagers())
    return false;

  unc::tclib::TcLibModule *tclib = GetTcLibModule();
  PFC_ASSERT(tclib != NULL);
  if (tclib == NULL) {
    UPLL_LOG_ERROR("Unable to get tclib module");
    return false;
  }
  TcLibIntfImpl *tlii = new TcLibIntfImpl(this);
  unc::tclib::TcApiCommonRet tacr = tclib->TcLibRegisterHandler(tlii);
  if (tacr != unc::tclib::TC_API_COMMON_SUCCESS) {
    UPLL_LOG_ERROR("Failed to register handler with tclib");
    delete tlii;
    return false;
  }

  return true;
}

const unc::capa::CapaIntf *UpllConfigMgr::GetCapaInterface() {
  unc::capa::CapaIntf *capa = reinterpret_cast<unc::capa::CapaIntf *>(
      pfc::core::Module::getInstance("capa"));
  if (capa == NULL) {
    pfc_log_fatal("CapaModule is not found");
  }
  return capa;
}

unc::tclib::TcLibModule *UpllConfigMgr::GetTcLibModule() {
  unc::tclib::TcLibModule *tclib = static_cast<unc::tclib::TcLibModule *>(
      pfc::core::Module::getInstance("tclib"));
  if (tclib == NULL) {
    pfc_log_fatal("TcLibModule is not found");
  }
  return tclib;
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

  // Take configuration locks
  if (TakeConfigLock(msghdr->operation, msghdr->datatype) == false) {
    msghdr->result_code = UPLL_RC_ERR_GENERIC;
    UPLL_LOG_DEBUG("Failed to take config lock.");
    return 0;
  }

  // Open Dal Connection
  DalOdbcMgr *dom = NULL;
  switch (msghdr->operation) {
    case UNC_OP_CREATE:
    case UNC_OP_UPDATE:
    case UNC_OP_DELETE:
    case UNC_OP_RENAME:
      dom = &config_rw_dom_;
      break;
    case UNC_OP_READ:
    case UNC_OP_READ_NEXT:
    case UNC_OP_READ_BULK:
    case UNC_OP_READ_SIBLING_BEGIN:
    case UNC_OP_READ_SIBLING:
    case UNC_OP_READ_SIBLING_COUNT:
      if (msghdr->datatype == UPLL_DT_CANDIDATE) {
        dom = &config_ro_dom_;
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
    if (UPLL_RC_SUCCESS != (urc = DalOpen(&dom, false, __FUNCTION__))) {
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
      urc = momgr->ReadSiblingCount(msghdr, first_ckv, dom);
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

  msghdr->result_code = urc;
  upll_rc_t db_urc = DalClose(dom, (urc == UPLL_RC_SUCCESS), __FUNCTION__);
  if (urc == UPLL_RC_SUCCESS) {
     msghdr->result_code = urc = db_urc;
  }

  if ((urc == UPLL_RC_SUCCESS) && (msghdr->datatype == UPLL_DT_CANDIDATE) &&
      ((msghdr->operation == UNC_OP_CREATE) ||
       (msghdr->operation == UNC_OP_UPDATE) ||
       (msghdr->operation == UNC_OP_DELETE))) {
    candidate_dirty_qc_lock_.lock();
    candidate_dirty_qc_ = true;
    candidate_dirty_qc_lock_.unlock();
  }

  // Release configuration locks
  if (ReleaseConfigLock(msghdr->operation, msghdr->datatype) == false) {
    UPLL_LOG_ERROR("Failed to release config lock.");
  }

  UPLL_LOG_TRACE("In UpllConfigMgr::ServiceHandler "
                 " service=%u\nResponse: %s\n%s",
                 service, IpcUtil::IpcResponseToStr(*msghdr).c_str(),
                 first_ckv->ToStrAll().c_str());

  return 0;
}

bool UpllConfigMgr::BuildKeyTree() {
  UPLL_FUNC_TRACE;
  // Init Regualr Config KeyType Tree
  if (cktt_.AddKeyType(UNC_KT_ROOT, UNC_KT_FLOWLIST) &&
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
          // skip UNC_KT_VTN_DATAFLOW_CONTROLLER
          cktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VBRIDGE) &&
          (
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
          iktt_.AddKeyType(UNC_KT_VTN, UNC_KT_VLINK)
      )
     ) {
  } else {
    return false;
  }

  iktt_.PrepareOrderedList();

  kt_name_map_[UNC_KT_ROOT] = "ROOT";
  kt_name_map_[UNC_KT_FLOWLIST] = "FLOWLIST";
  kt_name_map_[UNC_KT_FLOWLIST_ENTRY] = "FLOWLIST_ENTRY";
  kt_name_map_[UNC_KT_POLICING_PROFILE] = "POLICING_PROFILE";
  kt_name_map_[UNC_KT_POLICING_PROFILE_ENTRY] = "POLICING_PROFILE_ENTRY";
  kt_name_map_[UNC_KT_PATHMAP_ENTRY] = "PATHMAP_ENTRY";
  kt_name_map_[UNC_KT_PATHMAP_CONTROLLER] = "PATHMAP_CONTROLLER";
  kt_name_map_[UNC_KT_VTNSTATION_CONTROLLER] = "VTNSTATION_CONTROLLER";
  kt_name_map_[UNC_KT_VTN] = "VTN";
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
  kt_name_map_[UNC_KT_VUNKNOWN] = "VUNKNOWN";
  kt_name_map_[UNC_KT_VUNK_IF] = "VUNK_IF";
  kt_name_map_[UNC_KT_VTEP] = "VTEP";
  kt_name_map_[UNC_KT_VTEP_IF] = "VTEP_IF";
  kt_name_map_[UNC_KT_VTEP_GRP] = "VTEP_GRP";
  kt_name_map_[UNC_KT_VTEP_GRP_MEMBER] = "VTEP_GRP_MEMBER";
  kt_name_map_[UNC_KT_VTUNNEL] = "VTUNNEL";
  kt_name_map_[UNC_KT_VTUNNEL_IF] = "VTUNNEL_IF";
  kt_name_map_[UNC_KT_VLINK] = "VLINK";

  return true;
}

void UpllConfigMgr::DumpKeyTree() {
  unc_key_type_t kt;

  UPLL_LOG_TRACE("--- KeyTree Preorder Traversal ---");
  const std::list<unc_key_type_t> *pre_list = cktt_.get_preorder_list();
  std::list<unc_key_type_t>::const_iterator pre_it = pre_list->begin();
  while (pre_it != pre_list->end()) {
    kt = *pre_it;
    if (kt_name_map_.count(kt) > 0) {
      UPLL_LOG_TRACE("--- KT: %s", kt_name_map_[kt].c_str());
    }
    pre_it++;
  }
  UPLL_LOG_TRACE("--- KeyTree Postorder Traversal ---");
  const std::list<unc_key_type_t> *post_list =
      cktt_.get_reverse_postorder_list();
  std::list<unc_key_type_t>::const_iterator post_it = post_list->begin();
  while (post_it != post_list->end()) {
    kt = *post_it;
    if (kt_name_map_.count(kt) > 0) {
      UPLL_LOG_TRACE("--- KT: %s", kt_name_map_[kt].c_str());
    }
    post_it++;
  }
}

bool UpllConfigMgr::CreateMoManagers() {
  UPLL_FUNC_TRACE;

  namespace uuk = unc::upll::kt_momgr;

  upll_kt_momgrs_[UNC_KT_VTN] =
      upll_kt_momgrs_[UNC_KT_VTNSTATION_CONTROLLER] =
      upll_kt_momgrs_[UNC_KT_VTN_MAPPING_CONTROLLER] = new uuk::VtnMoMgr();
  upll_kt_momgrs_[UNC_KT_VBRIDGE] = new uuk::VbrMoMgr();
  upll_kt_momgrs_[UNC_KT_VBR_VLANMAP] = new uuk::VlanMapMoMgr();
  upll_kt_momgrs_[UNC_KT_VBR_NWMONITOR] = new uuk::NwMonitorMoMgr();
  upll_kt_momgrs_[UNC_KT_VBR_NWMONITOR_HOST] = new uuk::NwMonitorHostMoMgr();
  upll_kt_momgrs_[UNC_KT_VBR_IF] = new uuk::VbrIfMoMgr();
  upll_kt_momgrs_[UNC_KT_VROUTER] = new uuk::VrtMoMgr();
  upll_kt_momgrs_[UNC_KT_VRT_IPROUTE] = new uuk::IpRouteMoMgr();
  upll_kt_momgrs_[UNC_KT_DHCPRELAY_SERVER] = new uuk::DhcpRelayServerMoMgr();
  upll_kt_momgrs_[UNC_KT_DHCPRELAY_IF] = new uuk::DhcpRelayIfMoMgr();
  upll_kt_momgrs_[UNC_KT_VRT_IF] = new uuk::VrtIfMoMgr();
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
  return true;
}


upll_rc_t UpllConfigMgr::ValidSession(uint32_t clnt_sess_id,
                                      uint32_t config_id) {
  UPLL_FUNC_TRACE;
  unc::tclib::TcLibModule *tclib = GetTcLibModule();
  if (!tclib)
    return UPLL_RC_ERR_GENERIC;

  uint32_t err = tclib->TcLibValidateUpdateMsg(clnt_sess_id, config_id);
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
    urc = ValidSession(msghdr.clnt_sess_id, msghdr.config_id);
    if (urc != UPLL_RC_SUCCESS) {
      UPLL_LOG_DEBUG("Invalid session_id %u and/or config_id %u in IPC request."
                    " Err: %d", msghdr.clnt_sess_id, msghdr.config_id, urc);
      return urc;
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
                                      ConfigLock::LockType *lck_type_2) {
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
        return true;
      } else if (datatype == UPLL_DT_IMPORT) {
        *lck_dt_1 = UPLL_DT_IMPORT;
        *lck_type_1 = ConfigLock::CFG_WRITE_LOCK;
        *lck_dt_2 = UPLL_DT_RUNNING;
        *lck_type_2 = ConfigLock::CFG_READ_LOCK;
        return true;
      } else if (datatype == UPLL_DT_AUDIT) {
        *lck_dt_1 = UPLL_DT_AUDIT;
        *lck_type_1 = ConfigLock::CFG_WRITE_LOCK;
        *lck_dt_2 = UPLL_DT_RUNNING;
        *lck_type_2 = ConfigLock::CFG_READ_LOCK;
        return true;
      }
      return false;
      break;
    case UNC_OP_RENAME:
      if (datatype == UPLL_DT_IMPORT) {
        *lck_dt_1 = UPLL_DT_IMPORT;
        *lck_type_1 = ConfigLock::CFG_WRITE_LOCK;
        *lck_dt_2 = UPLL_DT_RUNNING;
        *lck_type_2 = ConfigLock::CFG_READ_LOCK;
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

  if (datatype == UPLL_DT_STATE)
    datatype = UPLL_DT_RUNNING;

  if (FindRequiredLocks(oper, datatype,
                        &lck_dt_1, &lck_type_1,
                        &lck_dt_2, &lck_type_2)) {
    if (lck_dt_2 == UPLL_DT_INVALID)
      return cfg_lock_.Lock(lck_dt_1, lck_type_1);
    else
      return cfg_lock_.Lock(lck_dt_1, lck_type_1, lck_dt_2, lck_type_2);
  }
  return false;
}

bool UpllConfigMgr::ReleaseConfigLock(unc_keytype_operation_t oper,
                                      upll_keytype_datatype_t datatype) {
  UPLL_FUNC_TRACE;
  upll_keytype_datatype_t lck_dt_1 = UPLL_DT_INVALID;
  upll_keytype_datatype_t lck_dt_2 = UPLL_DT_INVALID;
  ConfigLock::LockType lck_type_1, lck_type_2;

  if (datatype == UPLL_DT_STATE)
    datatype = UPLL_DT_RUNNING;

  if (FindRequiredLocks(oper, datatype,
                        &lck_dt_1, &lck_type_1,
                        &lck_dt_2, &lck_type_2)) {
    if (lck_dt_2 == UPLL_DT_INVALID)
      return cfg_lock_.Unlock(lck_dt_1);
    else
      return cfg_lock_.Unlock(lck_dt_1, lck_dt_2);
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
                                        uint32_t operation) {
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
  upll_rc_t urc = ValidSession(sess_id, config_id);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Invalid session_id %u and/or config_id %u in IPC request,"
                  " Err: %d, %s not allowed for controller %s",
                  sess_id, config_id, urc, import_operation, ctrlr_id);
    return urc;
  }

  if (operation == UPLL_CLEAR_IMPORT_CONFIG_OP)
    return UPLL_RC_SUCCESS;  // the other below checks are not required

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
  if (ctrlr_type != UNC_CT_PFC) {
    UPLL_LOG_INFO("Import is not allowed for controller type %d."
                  " Cannot do %s for %s ",
                  ctrlr_type, import_operation, ctrlr_id);
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

  bool candidate_dirty = true;
  // Note: caller supposed to take the lock on the database
  urc = IsCandidateDirtyNoLock(&candidate_dirty);
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
  // Note: caller supposed to take the lock on the database
  urc = IsKeyInUseNoLock(UPLL_DT_RUNNING, &ckv, &ctr_in_use);
  if (urc != UPLL_RC_SUCCESS) {
    UPLL_LOG_INFO("Failed to find if controller is in running config. Urc=%d."
                   "Cannot do %s for %s", urc, import_operation, ctrlr_id);
    return urc;
  }
  if (ctr_in_use == true) {
    UPLL_LOG_INFO("Controller is already configured. Cannot do %s for %s",
                  import_operation, ctrlr_id);
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllConfigMgr::ImportCtrlrConfig(const char *ctrlr_id,
                                           upll_keytype_datatype_t dest_dt) {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = UPLL_RC_SUCCESS;

  DalOdbcMgr *dom = &import_rw_dom_;      // Dal Connection

  key_root_t *root_key = reinterpret_cast<key_root_t *>(
      ConfigKeyVal::Malloc(sizeof(key_root_t)));


  ConfigKeyVal *bulkreq_ckv = new ConfigKeyVal(UNC_KT_ROOT,
                                               IpctSt::kIpcStKeyRoot,
                                               root_key);

  bool read_bulk_done = false;
  while (!read_bulk_done) {
    IpcRequest bulk_req;
    IpcResponse bulk_resp;
    bzero(&bulk_req, sizeof(IpcRequest));
    bzero(&bulk_resp, sizeof(IpcResponse));

    // bulk_req.header.clnt_sess_id = 0;
    // bulk_req.header.config_id = 0;
    bulk_req.header.operation = UNC_OP_READ_BULK;
    bulk_req.header.rep_count = 0xFFFFFFFF;  // Driver might be ignoring this
    bulk_req.header.option1 = UNC_OPT1_NORMAL;
    bulk_req.header.option2 = UNC_OPT2_NONE;
    bulk_req.header.datatype = UPLL_DT_RUNNING;
    bulk_req.ckv_data = bulkreq_ckv;
    if (IpcUtil::SendReqToDriver(ctrlr_id, NULL, NULL, 0,
                                 &bulk_req, false, &bulk_resp) == false) {
      UPLL_LOG_DEBUG("Failed to send request to driver for %s", ctrlr_id);
      urc = UPLL_RC_ERR_GENERIC;
      if (bulk_req.ckv_data) {
        delete bulk_req.ckv_data;
        bulk_req.ckv_data = NULL;
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
      if (upll_kt_momgrs_.count(kt) > 0) {
        MoManager *momgr = upll_kt_momgrs_[kt];
        // In import-readbulk driver cannot return domain id in message header
        // as there will be more than one VTN. A VTN will be sent in the
        // response only if a VNODE is present for the VTN. VNODE val structure
        // will have domain id.
        urc = momgr->CreateImportMo(&one_reqhdr, one_ckv, dom, ctrlr_id, NULL);
        delete one_ckv;
        one_ckv = NULL;
        if (urc != UPLL_RC_SUCCESS) {
          UPLL_LOG_DEBUG("CreateImportMo failed. Error = %d", urc);
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

  UPLL_LOG_DEBUG("Import configuration status. Urc=%d", urc);
  upll_rc_t close_urc = DalClose(dom, (urc == UPLL_RC_SUCCESS), __FUNCTION__);
  if (urc == UPLL_RC_SUCCESS) {
    urc = close_urc;
  }

  return urc;
}

upll_rc_t UpllConfigMgr::StartImport(const char *ctrlr_id, uint32_t sess_id,
                                     uint32_t config_id) {
  UPLL_FUNC_TRACE;

  pfc::core::ScopedMutex import_lock(import_mutex_lock_);

  // Validate if import is already in progress
  if (import_in_progress_ == true) {
    UPLL_LOG_INFO("Import is in progress for %s. Cannot do import for %s",
                   import_ctrlr_id_.c_str(), ctrlr_id);
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }

  ScopedConfigLock scfg_lock(cfg_lock_,
                             UPLL_DT_IMPORT, ConfigLock::CFG_WRITE_LOCK,
                             UPLL_DT_CANDIDATE, ConfigLock::CFG_READ_LOCK);

  // Take read lock on running for doing IsCandidateDirty and IsKeyInUse check
  // in ValidateImport
  ScopedConfigLock scfg_lock2(cfg_lock_,
                             UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);


  upll_rc_t urc = ValidateImport(sess_id, config_id, ctrlr_id,
                                 UPLL_IMPORT_CTRLR_CONFIG_OP);
  if (urc != UPLL_RC_SUCCESS)
    return urc;


  // All checks are over. Let us import configuration.
  import_in_progress_ = true;
  import_ctrlr_id_ = ctrlr_id;

  urc = ImportCtrlrConfig(ctrlr_id, UPLL_DT_IMPORT);

  if (urc != UPLL_RC_SUCCESS) {
    import_in_progress_ = false;
    import_ctrlr_id_ = "";
  }

  return urc;
}

upll_rc_t UpllConfigMgr::OnMerge(uint32_t sess_id, uint32_t config_id) {
  UPLL_FUNC_TRACE;
  upll_rc_t urc;

  // Validate if import is in progress
  pfc::core::ScopedMutex lock(import_mutex_lock_);
  if (import_in_progress_ == false) {
    UPLL_LOG_INFO("Import is not in progress. Merge cannot be done");
    return UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME;
  }

  ScopedConfigLock scfg_lock(cfg_lock_,
                             UPLL_DT_IMPORT, ConfigLock::CFG_READ_LOCK,
                             UPLL_DT_CANDIDATE, ConfigLock::CFG_WRITE_LOCK);

  // Take read lock on running for doing IsCandidateDirty and IsKeyInUse check
  // in ValidateImport
  ScopedConfigLock scfg_lock2(cfg_lock_,
                             UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  DalOdbcMgr *dom = &import_rw_dom_;

  urc = ValidateImport(sess_id, config_id, import_ctrlr_id_.c_str(),
                       UPLL_MERGE_IMPORT_CONFIG_OP);

  if (urc == UPLL_RC_SUCCESS)
    urc = MergeValidate();  // validate if merge conflicts with existing config
  if (urc == UPLL_RC_SUCCESS)
    urc = MergeImportToCandidate();  // merge import config to candidate config

  upll_rc_t db_urc = DalClose(dom, (urc == UPLL_RC_SUCCESS), __FUNCTION__);
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }

  if (urc == UPLL_RC_SUCCESS) {
    candidate_dirty_qc_lock_.lock();
    candidate_dirty_qc_ = true;
    candidate_dirty_qc_lock_.unlock();
  }

  return urc;
}

upll_rc_t UpllConfigMgr::MergeValidate() {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = UPLL_RC_SUCCESS;
  unc_key_type_t kt;

  DalOdbcMgr *dom = &import_rw_dom_;

  const std::list<unc_key_type_t> *pre_list = cktt_.get_preorder_list();
  for (std::list<unc_key_type_t>::const_iterator pre_it = pre_list->begin();
       pre_it != pre_list->end(); pre_it++) {
    kt = *pre_it;
    bool flag = false;
    if (upll_kt_momgrs_.count(kt) > 0) {
       unc_key_type_t child_key[]= { UNC_KT_VBRIDGE, UNC_KT_VBR_VLANMAP,
                    UNC_KT_VBR_NWMONITOR, UNC_KT_VBR_NWMONITOR_HOST,
                    UNC_KT_VBR_IF, UNC_KT_VROUTER, UNC_KT_VRT_IPROUTE,
                    UNC_KT_DHCPRELAY_SERVER, UNC_KT_DHCPRELAY_IF,
                    UNC_KT_VRT_IF, UNC_KT_VUNKNOWN, UNC_KT_VUNK_IF,
                    UNC_KT_VTEP, UNC_KT_VTEP_IF, UNC_KT_VTEP_GRP,
                    UNC_KT_VTEP_GRP_MEMBER, UNC_KT_VTUNNEL,
                    UNC_KT_VTUNNEL_IF, UNC_KT_VLINK
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
      MoManager *momgr = upll_kt_momgrs_[kt];
      ConfigKeyVal conflict_ckv(kt);
      urc = momgr->MergeValidate(kt, import_ctrlr_id_.c_str(),
                                 &conflict_ckv, dom);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("MergeValidate failed %d", urc);
        break;
      }
    }
  }
  return urc;
}

upll_rc_t UpllConfigMgr::MergeImportToCandidate() {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = UPLL_RC_SUCCESS;
  unc_key_type_t kt;

  DalOdbcMgr *dom = &import_rw_dom_;

  const std::list<unc_key_type_t> *pre_list = cktt_.get_preorder_list();
  for (std::list<unc_key_type_t>::const_iterator pre_it = pre_list->begin();
       pre_it != pre_list->end(); pre_it++) {
    kt = *pre_it;
    if (upll_kt_momgrs_.count(kt) > 0) {
      MoManager *momgr = upll_kt_momgrs_[kt];
      urc = momgr->MergeImportToCandidate(kt, import_ctrlr_id_.c_str(), dom);
      if (urc != UPLL_RC_SUCCESS) {
        UPLL_LOG_DEBUG("MergeImportToCandidate failed %d", urc);
        break;
      }
    }
  }

#if 0
  upll_rc_t db_urc = DalClose(dom, (urc == UPLL_RC_SUCCESS), __FUNCTION__);

  candidate_dirty_qc_lock_.lock();
  candidate_dirty_qc_ = true;
  candidate_dirty_qc_lock_.unlock();
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }
#endif
  return urc;
}

upll_rc_t UpllConfigMgr::IsCandidateDirty(bool *dirty) {
  UPLL_FUNC_TRACE;

  ScopedConfigLock scfg_lock(cfg_lock_,
                             UPLL_DT_CANDIDATE, ConfigLock::CFG_READ_LOCK,
                             UPLL_DT_RUNNING, ConfigLock::CFG_WRITE_LOCK);
  return IsCandidateDirtyNoLock(dirty);
}

upll_rc_t UpllConfigMgr::IsCandidateDirtyNoLock(bool *dirty) {
  UPLL_FUNC_TRACE;
  upll_rc_t urc = UPLL_RC_SUCCESS;
  unc_key_type_t kt;
  if (dirty == NULL) {
    UPLL_LOG_DEBUG("Null argument: dirty");
    return UPLL_RC_ERR_GENERIC;
  }

  candidate_dirty_qc_lock_.lock();
  *dirty = candidate_dirty_qc_;
  candidate_dirty_qc_lock_.unlock();
  if (*dirty == false) {
    return UPLL_RC_SUCCESS;
  }

  DalOdbcMgr *dom = NULL;
  if (UPLL_RC_SUCCESS != (urc = DalOpen(&dom, false, __FUNCTION__))) {
    return urc;
  }

  *dirty = false;

  const std::list<unc_key_type_t> *pre_list = cktt_.get_preorder_list();
  for (std::list<unc_key_type_t>::const_iterator pre_it = pre_list->begin();
       pre_it != pre_list->end(); pre_it++) {
    kt = *pre_it;
    if (upll_kt_momgrs_.count(kt) > 0) {
      MoManager *momgr = upll_kt_momgrs_[kt];
      urc = momgr->IsCandidateDirty(kt, dirty, dom);
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

  upll_rc_t db_urc = DalClose(dom, (urc == UPLL_RC_SUCCESS), __FUNCTION__);
  if (urc == UPLL_RC_SUCCESS) {
    urc = db_urc;
  }

  return urc;
}

upll_rc_t UpllConfigMgr::IsKeyInUse(upll_keytype_datatype_t datatype,
                                    const ConfigKeyVal *ckv, bool *in_use) {
  UPLL_FUNC_TRACE;
  ScopedConfigLock scfg_lock(cfg_lock_, datatype, ConfigLock::CFG_READ_LOCK);
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
  MoManager *momgr = NULL;
  switch (ckv->get_key_type()) {
    case UNC_KT_CONTROLLER:
      momgr = GetMoManager(UNC_KT_VTN);
      break;
    case UNC_KT_BOUNDARY:
      momgr = GetMoManager(UNC_KT_VLINK);
      break;
    case UNC_KT_PORT:
    default:
      return UPLL_RC_ERR_NO_SUCH_NAME;
  }
  if (momgr) {
    DalOdbcMgr *dom = NULL;
    if (UPLL_RC_SUCCESS != (urc = DalOpen(&dom, false, __FUNCTION__))) {
      return urc;
    }

    urc = momgr->IsKeyInUse(datatype, ckv, in_use, dom);

    upll_rc_t db_urc = DalClose(dom, (urc == UPLL_RC_SUCCESS), __FUNCTION__);
    if (urc == UPLL_RC_SUCCESS) {
      urc = db_urc;
    }
  } else {
    urc = UPLL_RC_ERR_GENERIC;
  }

  return urc;
}

static upll_rc_t ConvertDalResultCode(uudal::DalResultCode drc) {
  switch (drc) {
    case uudal::kDalRcSuccess:
      return UPLL_RC_SUCCESS;
    case uudal::kDalRcConnNotEstablished:
    case uudal::kDalRcConnNotAvailable:
    case uudal::kDalRcConnTimeOut:
    case uudal::kDalRcQueryTimeOut:
      return UPLL_RC_ERR_RESOURCE_DISCONNECTED;
    case uudal::kDalRcTxnError:
    case uudal::kDalRcAccessViolation:
      return UPLL_RC_ERR_DB_ACCESS;
    case uudal::kDalRcNotDisconnected:
    case uudal::kDalRcInvalidConnHandle:
    case uudal::kDalRcInvalidCursor:
    case uudal::kDalRcDataError:
    case uudal::kDalRcRecordAlreadyExists:
    case uudal::kDalRcRecordNotFound:
    case uudal::kDalRcRecordNoMore:
    case uudal::kDalRcMemoryError:
    case uudal::kDalRcInternalError:
    case uudal::kDalRcGeneralError:
    // default:
      return UPLL_RC_ERR_GENERIC;
  }
  return UPLL_RC_ERR_GENERIC;
}

upll_rc_t UpllConfigMgr::DalOpen(DalOdbcMgr **dom, bool read_write_conn,
                                 const char *caller) {
  UPLL_FUNC_TRACE;

  uudal::DalResultCode drc;

  uudal::DalConnType conn_type = ((read_write_conn) ? uudal::kDalConnReadWrite :
                                  uudal::kDalConnReadOnly);
  bool dom_allocated = false;
  if ((*dom != &config_rw_dom_) && (*dom != &import_rw_dom_) &&
      (*dom != &alarm_rw_dom_) && (*dom != &config_ro_dom_)) {
    *dom = new DalOdbcMgr();
    dom_allocated = true;
  }

  // Initialize
  drc = (*dom)->Init();
  if (drc != uudal::kDalRcSuccess) {
    if (read_write_conn) {
      pfc_log_fatal("%s: Failed to initialize DalOdbcMgr. Err=%d", caller, drc);
    } else {
      UPLL_LOG_ERROR("%s: Failed to initialize DalOdbcMgr. Err=%d", caller,
                     drc);
    }
    if (dom_allocated) {
      delete (*dom);
      *dom = NULL;
    }
    return ConvertDalResultCode(drc);
  } else {
    UPLL_LOG_TRACE("%s: DalOdbcMgr init successful.", caller);
  }

  // Connect to DB
  drc = (*dom)->ConnectToDb(conn_type);
  if (drc != uudal::kDalRcSuccess) {
    if (read_write_conn) {
      pfc_log_fatal("%s: Failed to connect to database. Err=%d", caller, drc);
    } else {
      UPLL_LOG_ERROR("%s: Failed to connect to database. Err=%d", caller, drc);
    }
    if (dom_allocated) {
      delete (*dom);
      *dom = NULL;
    }
    return ConvertDalResultCode(drc);
  } else {
    UPLL_LOG_TRACE("%s: Connected to database.", caller);
  }

  return UPLL_RC_SUCCESS;
}

upll_rc_t UpllConfigMgr::DalClose(DalOdbcMgr *dom, bool commit,
                                  const char *caller) {
  UPLL_FUNC_TRACE;

  uudal::DalResultCode drc;
  upll_rc_t urc = UPLL_RC_SUCCESS;

  // Commit or Rollback is required only read-write connections
  if (dom->get_conn_type() == uudal::kDalConnReadWrite) {
    if (commit) {
      // Commit the transaction
      drc = dom->CommitTransaction();
      if (drc != uudal::kDalRcSuccess) {
        UPLL_LOG_ERROR("%s: Failed to commit DB transaction. Err=%d",
                       caller, drc);
        urc = ConvertDalResultCode(drc);
      } else {
        UPLL_LOG_TRACE("%s: Committed the DB transaction.", caller);
        urc = UPLL_RC_SUCCESS;
      }
    } else {
      // Rollback the transaction
      drc = dom->RollbackTransaction();
      if (drc != uudal::kDalRcSuccess) {
        UPLL_LOG_ERROR("%s: Failed to rollback DB transaction. Err=%d",
                       caller, drc);
        urc = ConvertDalResultCode(drc);
      } else {
        UPLL_LOG_TRACE("%s: Rolledback the DB transaction.", caller);
        urc = UPLL_RC_SUCCESS;
      }
    }
  }

  // disconnect and delete dom instance for dynamic connections .
  if ((dom != &config_rw_dom_) && (dom != &import_rw_dom_) &&
      (dom != &alarm_rw_dom_) && (dom != &config_ro_dom_)) {
    // Disconnect from database; on error do not change urc.
    drc = dom->DisconnectFromDb();
    if (drc != uudal::kDalRcSuccess) {
      UPLL_LOG_ERROR("%s: Failed to disconnect from database. Err=%d",
                     caller, drc);
    } else {
      UPLL_LOG_TRACE("%s: Disconnected from database.", caller);
    }

    delete dom;
  }

  return urc;
}

upll_rc_t UpllConfigMgr::DalOpenInitialConnections() {
  UPLL_FUNC_TRACE;
  pfc::core::ScopedMutex lock(rw_dom_mutex_lock_);
  upll_rc_t urc = UPLL_RC_SUCCESS;

  DalOdbcMgr *dom = &config_rw_dom_;
  if (UPLL_RC_SUCCESS != (urc = DalOpen(&dom, true, __FUNCTION__))) {
    return urc;
  }
  dom = &config_ro_dom_;
  if (UPLL_RC_SUCCESS != (urc = DalOpen(&dom, true, __FUNCTION__))) {
    return urc;
  }
  dom = &import_rw_dom_;
  if (UPLL_RC_SUCCESS != (urc = DalOpen(&dom, true, __FUNCTION__))) {
    return urc;
  }
  dom = &alarm_rw_dom_;
  if (UPLL_RC_SUCCESS != (urc = DalOpen(&dom, true, __FUNCTION__))) {
    return urc;
  }

  return urc;
}

upll_rc_t UpllConfigMgr::DalCloseInitialConnections() {
  UPLL_FUNC_TRACE;
  pfc::core::ScopedMutex lock(rw_dom_mutex_lock_);
  upll_rc_t urc = UPLL_RC_SUCCESS;
  uudal::DalResultCode drc;

  drc = config_rw_dom_.DisconnectFromDb();
  if (drc != uudal::kDalRcSuccess) {
    UPLL_LOG_ERROR("Failed to disconnect config_rw_dom_ from database. Err=%d",
                   drc);
    urc = UPLL_RC_ERR_GENERIC;
  }
  drc = config_ro_dom_.DisconnectFromDb();
  if (drc != uudal::kDalRcSuccess) {
    UPLL_LOG_ERROR("Failed to disconnect config_ro_dom_ from database. Err=%d",
                   drc);
    urc = UPLL_RC_ERR_GENERIC;
  }
  drc = import_rw_dom_.DisconnectFromDb();
  if (drc != uudal::kDalRcSuccess) {
    UPLL_LOG_ERROR("Failed to disconnect import_rw_dom_ from database. Err=%d",
                   drc);
    urc = UPLL_RC_ERR_GENERIC;
  }
  drc = alarm_rw_dom_.DisconnectFromDb();
  if (drc != uudal::kDalRcSuccess) {
    UPLL_LOG_ERROR("Failed to disconnect alarm_rw_dom_ from database. Err=%d",
                   drc);
    urc = UPLL_RC_ERR_GENERIC;
  }

  return urc;
}

void UpllConfigMgr::OnPathFaultAlarm(const char *ctrlr_name,
                                     const char *domain_name,
                                     std::vector<std::string> &ingress_ports,
                                     std::vector<std::string> &egress_ports,
                                     bool alarm_asserted) {
  UPLL_FUNC_TRACE;
  ScopedConfigLock lock(cfg_lock_, UPLL_DT_RUNNING, ConfigLock::CFG_WRITE_LOCK);
  uuk::VbrIfMoMgr *mgr = reinterpret_cast<uuk::VbrIfMoMgr *>
                    (GetMoManager(UNC_KT_VBR_IF));
  if (mgr != NULL) {
    upll_rc_t urc;
    DalOdbcMgr *dom = &alarm_rw_dom_;
    urc = mgr->PathFaultHandler(ctrlr_name, domain_name, ingress_ports,
                                egress_ports, alarm_asserted, dom);
    DalClose(dom, (urc == UPLL_RC_SUCCESS), __FUNCTION__);
  }
}

void UpllConfigMgr::OnControllerStatusChange(const char *ctrlr_name,
                                             bool operstatus) {
  UPLL_FUNC_TRACE;
  ScopedConfigLock lock(cfg_lock_, UPLL_DT_RUNNING, ConfigLock::CFG_WRITE_LOCK);
  uuk::VtnMoMgr *mgr = reinterpret_cast<uuk::VtnMoMgr *>
      (GetMoManager(UNC_KT_VTN));
  if (mgr != NULL) {
    upll_rc_t urc;
    DalOdbcMgr *dom = &alarm_rw_dom_;
    urc = mgr->ControllerStatusHandler(reinterpret_cast<uint8_t*>(
            const_cast<char*>(ctrlr_name)), dom, operstatus);
    DalClose(dom, (urc == UPLL_RC_SUCCESS), __FUNCTION__);
  }
}

void UpllConfigMgr::OnLogicalPortStatusChange(const char *ctrlr_name,
                                              const char *domain_name,
                                              const char *logical_port_id,
                                              bool oper_status) {
  UPLL_FUNC_TRACE;
  ScopedConfigLock lock(cfg_lock_, UPLL_DT_RUNNING, ConfigLock::CFG_WRITE_LOCK);
  uuk::VbrIfMoMgr *mgr = reinterpret_cast<uuk::VbrIfMoMgr *>
      (GetMoManager(UNC_KT_VBR_IF));
  if (mgr != NULL) {
    upll_rc_t urc;
    DalOdbcMgr *dom = &alarm_rw_dom_;
    urc = mgr->PortStatusHandler(ctrlr_name, domain_name, logical_port_id,
                                 oper_status, dom);
    DalClose(dom, (urc == UPLL_RC_SUCCESS), __FUNCTION__);
  }
}

void UpllConfigMgr::OnBoundaryStatusChange(const char *boundary_id,
                                           bool oper_status) {
  UPLL_FUNC_TRACE;
  ScopedConfigLock lock(cfg_lock_, UPLL_DT_RUNNING, ConfigLock::CFG_WRITE_LOCK);
#if 1
  uuk::VlinkMoMgr *mgr = reinterpret_cast<uuk::VlinkMoMgr *>
      (GetMoManager(UNC_KT_VLINK));
  if (mgr != NULL) {
    upll_rc_t urc;
    DalOdbcMgr *dom = &alarm_rw_dom_;
    urc = mgr->BoundaryStatusHandler(
        reinterpret_cast<uint8_t*>(const_cast<char*>(boundary_id)),
        oper_status, dom);
    DalClose(dom, (urc == UPLL_RC_SUCCESS), __FUNCTION__);
  }
#else
  UPLL_LOG_DEBUG("Recevied boundary %s oper status %d\n",boundary_id,oper_status);
#endif
}

void UpllConfigMgr::OnPolicerFullAlarm(
    string ctrlr_name, string domain_id, const key_vtn_t &key_vtn,
    const pfcdrv_policier_alarm_data_t &alarm_data, bool alarm_raised) {
  UPLL_FUNC_TRACE;
  namespace uuk = unc::upll::kt_momgr;
  ScopedConfigLock lock(cfg_lock_, UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  uuk::VbrPolicingMapMoMgr *mgr  = reinterpret_cast<uuk::VbrPolicingMapMoMgr*>
      (GetMoManager(UNC_KT_VBR_POLICINGMAP));
  if (mgr != NULL) {
    upll_rc_t urc;
    DalOdbcMgr *dom = NULL;
    if (UPLL_RC_SUCCESS != (urc = DalOpen(&dom, false, __FUNCTION__))) {
      return;
    }
    urc = mgr->OnPolicerFullAlarm(ctrlr_name, domain_id, key_vtn,
                                  alarm_data, alarm_raised, dom);
    DalClose(dom, (urc == UPLL_RC_SUCCESS), __FUNCTION__);
  }
}

void UpllConfigMgr::OnPolicerFailAlarm(
    string ctrlr_name, string domain_id, const key_vtn_t &key_vtn,
    const pfcdrv_policier_alarm_data_t &alarm_data, bool alarm_raised) {
  UPLL_FUNC_TRACE;
  namespace uuk = unc::upll::kt_momgr;
  ScopedConfigLock lock(cfg_lock_, UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);

  uuk::VbrPolicingMapMoMgr *mgr  = reinterpret_cast<uuk::VbrPolicingMapMoMgr*>
      (GetMoManager(UNC_KT_VBR_POLICINGMAP));

  if (mgr != NULL) {
    upll_rc_t urc;
    DalOdbcMgr *dom = NULL;
    if (UPLL_RC_SUCCESS != (urc = DalOpen(&dom, false, __FUNCTION__))) {
      return;
    }
    urc = mgr->OnPolicerFailAlarm(ctrlr_name, domain_id, key_vtn,
                                  alarm_data, alarm_raised, dom);
    DalClose(dom, (urc == UPLL_RC_SUCCESS), __FUNCTION__);
  }
}

void UpllConfigMgr::OnNwmonFaultAlarm(
    string ctrlr_name, string domain_id, const key_vtn_t &vtn_key,
    const pfcdrv_network_mon_alarm_data_t &alarm_data, bool alarm_raised) {
  UPLL_FUNC_TRACE;
  ScopedConfigLock lock(cfg_lock_, UPLL_DT_RUNNING, ConfigLock::CFG_READ_LOCK);
  uuk::NwMonitorMoMgr *nwm_mgr = reinterpret_cast<uuk::NwMonitorMoMgr*>
         (const_cast<MoManager*>(GetMoManager(UNC_KT_VBR_NWMONITOR)));
  if (nwm_mgr != NULL) {
    upll_rc_t urc;
    DalOdbcMgr *dom = &alarm_rw_dom_;
    urc = nwm_mgr->OnNwmonFault(ctrlr_name, domain_id, vtn_key, alarm_data,
                            alarm_raised, dom);
    DalClose(dom, (urc == UPLL_RC_SUCCESS), __FUNCTION__);
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
    message = "Operational status down";
    message_summary = "Operational status down";
    data.alarm_class = pfc::alarm::ALM_WARNING;
    data.alarm_kind = 1;   // assert alarm
  } else {
    message = "Operational status up";
    message_summary = "Operational status up";
    data.alarm_class = pfc::alarm::ALM_NOTICE;
    data.alarm_kind = 0;   // clear alarm
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
                                                                       // NOLINT
}  // namesapce config_momgr
}  // namesapce upll
}  // namesapce unc
