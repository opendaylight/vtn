/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UNC_UPLL_MOMGR_IMPL_H
#define UNC_UPLL_MOMGR_IMPL_H

#include <sys/time.h>
#include <functional>

#include <map>
#include <string>
#include <cstring>
#include <list>
#include <set>

#include "unc/keytype.h"
#include "unc/pfcdriver_include.h"
#include "unc/pfcdriver_ipc_enum.h"
#include "unc/upll_ipc_enum.h"
#include "ipc_util.hh"
#include "ipct_st.hh"
#include "uncxx/upll_log.hh"
#include "upll_util.hh"
#include "kt_util.hh"
#include "dal_defines.hh"
#include "dal_schema.hh"
#include "momgr_intf.hh"
#include "unc/upll_errno.h"
#include "upll_validation.hh"
#include "ctrlr_capa_defines.hh"
#include "ctrlr_mgr.hh"
#include "unc/uppl_common.h"
#include "config_mgr.hh"

namespace unc {
namespace upll {
namespace kt_momgr {

using std::list;
using std::multimap;
using std::string;
using std::set;
using std::map;

namespace uucfg = unc::upll::config_momgr;
namespace uuc = unc::upll::config_momgr;
namespace uud = unc::upll::dal;
namespace uudst = unc::upll::dal::schema::table;
namespace uui = unc::upll::ipc_util;
namespace uuu = unc::upll::upll_util;

using unc::upll::ipc_util::IpctSt;
using unc::upll::ipc_util::IpcResponse;
using unc::upll::ipc_util::ConfigKeyVal;
using unc::upll::ipc_util::ConfigVal;
using unc::upll::ipc_util::ConfigNotification;
using unc::upll::ipc_util::ConfigNotifier;
using unc::upll::ipc_util::IpcReqRespHeader;
using unc::upll::ipc_util::IpcRequest;
using unc::upll::config_momgr::CtrlrCommitStatus;
using unc::upll::config_momgr::CtrlrVoteStatus;
using unc::upll::config_momgr::KTxCtrlrAffectedState;
using unc::upll::config_momgr::MoManager;
using unc::upll::dal::DalBindInfo;
using unc::upll::dal::DalDmlIntf;
using unc::upll::dal::DalCursor;
using unc::upll::dal::DalResultCode;
using unc::upll::ipc_util::controller_domain;
using unc::upll::ipc_util::controller_domain_t;
using unc::upll::tx_update_util::TxUpdateUtil;

#define MAX_RENAME_FLAG_LEN 1
#define STAND_ALONE_VNODE 0xFFFFFFFF

#define MAX_LABEL_RANGE 0XFFFFFFFF
#define REMOTE_IF_DISCONNECT 0xFFFFFFFF
#define INVALID_MATCH_VALUE 0xFFFF

inline bool IsUnifiedVbr(uint8_t *ctrlr_id) {
  return (!strcmp(reinterpret_cast<char *>(ctrlr_id), "#"));
}

#define ResetValid(x, val) { \
  struct x *tval = reinterpret_cast<struct x *>(val);           \
  for (uint8_t i = 0; i < sizeof(tval->valid)/sizeof(tval->valid[0]); i++) \
    *reinterpret_cast<uint8_t *>(tval->valid + i)  = UNC_VF_INVALID; \
}

#define GetVal(ikey) \
  ((ikey) ? \
    (((ikey)->get_cfg_val()) ? \
      (((ikey)->get_cfg_val())->get_val()) \
    : NULL) \
  : NULL)

#define GetStateVal(ikey) \
  ((ikey) ? \
    (((ikey)->get_cfg_val()) ? \
      (((ikey)->get_cfg_val()->get_next_cfg_val()) ? \
        ((ikey)->get_cfg_val()->get_next_cfg_val()->get_val()) \
      : NULL) \
    : NULL) \
  : NULL)

// #define KEY_RESET(x) *(x) = ' '



// typedef list<string> ListStr;
// typedef list<int> ListInt;
typedef list<CtrlrCommitStatus *> CtrlrCommitStatusList;
typedef list<CtrlrVoteStatus *> CtrlrVoteStatusList;
// typedef multimap<uint8_t*, ConfigKeyVal *>ControllerKeyMap;
#define UNKNOWN_KT(keytype) ((keytype == UNC_KT_VUNKNOWN) || \
     (keytype == UNC_KT_VUNK_IF))

#define OVERLAY_KT(keytype)  ((keytype == UNC_KT_VTEP) || \
    (keytype == UNC_KT_VTEP_IF) || (keytype == UNC_KT_VTEP_GRP) || \
    (keytype == UNC_KT_VTEP_GRP_MEMBER) || (keytype == UNC_KT_VTUNNEL) ||\
    (keytype == UNC_KT_VTUNNEL_IF))

#define PORT_MAPPED_KEYTYPE(keytype) ((keytype == UNC_KT_VBR_IF) || \
    (keytype == UNC_KT_VTEP_IF) || (keytype == UNC_KT_VTUNNEL_IF) || \
    (keytype == UNC_KT_VTERM_IF))

#define VN_IF_KEYTYPE(keytype) ((keytype == UNC_KT_VBR_IF) || \
    (keytype == UNC_KT_VTEP_IF) || (keytype == UNC_KT_VTUNNEL_IF) || \
    (keytype == UNC_KT_VRT_IF) || (keytype == UNC_KT_VTERM_IF))

#define VNODE_KEYTYPE(keytype) ((keytype == UNC_KT_VBRIDGE) || \
    (keytype == UNC_KT_VROUTER) || (keytype == UNC_KT_VUNKNOWN) || \
    (keytype == UNC_KT_VTEP) || (keytype == UNC_KT_VTUNNEL) || \
    (keytype == UNC_KT_VTERMINAL))

// TODO(PCM): Needs change when Unified vBridge is implemented.
#define VIRTUAL_MODE_KT(keytype) ((keytype == UNC_KT_POLICING_PROFILE) \
         || (keytype == UNC_KT_POLICING_PROFILE_ENTRY) \
         || (keytype == UNC_KT_FLOWLIST) \
         || (keytype == UNC_KT_FLOWLIST_ENTRY) \
         || (keytype == UNC_KT_UNIFIED_NETWORK) \
         || (keytype == UNC_KT_UNW_LABEL) \
         || (keytype == UNC_KT_UNW_LABEL_RANGE) \
         || (keytype == UNC_KT_UNW_SPINE_DOMAIN))

#define GET_USER_DATA(ckey) { \
  void *user_data = (ckey)->get_user_data(); \
  if (!user_data)  { \
    user_data = malloc(sizeof(key_user_data)); \
    if (user_data) { \
      memset(user_data, 0, sizeof(key_user_data)); \
      (ckey)->set_user_data(user_data); \
    } \
  } \
}

#define SET_USER_DATA(ckey, skey) { \
  key_user_data_t *suser_data = \
    reinterpret_cast<key_user_data_t *>((skey)?(skey->get_user_data()):NULL); \
  if (suser_data)  {\
    GET_USER_DATA(ckey)      \
    key_user_data_t *user_data = \
                 reinterpret_cast<key_user_data_t *>(ckey->get_user_data()); \
    uuu::upll_strncpy(user_data->ctrlr_id, suser_data->ctrlr_id, \
                                             (kMaxLenCtrlrId+1)); \
    uuu::upll_strncpy(user_data->domain_id, suser_data->domain_id, \
                                             (kMaxLenDomainId+1)); \
    user_data->flags = suser_data->flags; \
  } \
}

// free the user data and set it to NULL
#define DEL_USER_DATA(ckey) {       \
  if ((ckey)->get_user_data()) {    \
    free((ckey)->get_user_data());  \
  }                                 \
  (ckey)->set_user_data(NULL);      \
}

#define SET_USER_DATA_CTRLR(ckey, ctrlr) { \
  GET_USER_DATA(ckey)      \
  if (ctrlr && strlen(reinterpret_cast<const char *>(ctrlr))) { \
    key_user_data *user_data = \
                 reinterpret_cast<key_user_data *>(ckey->get_user_data()); \
    uuu::upll_strncpy(user_data->ctrlr_id, (ctrlr), (kMaxLenCtrlrId+1)); \
  } \
}

#define SET_USER_DATA_DOMAIN(ckey, domain) { \
  GET_USER_DATA(ckey)      \
  if (domain && strlen(reinterpret_cast<const char *>(domain))) { \
    key_user_data *user_data = \
                 reinterpret_cast<key_user_data *>(ckey->get_user_data()); \
    uuu::upll_strncpy(user_data->domain_id, (domain), (kMaxLenDomainId+1)); \
  } \
}

#define SET_USER_DATA_CTRLR_DOMAIN(ckey, ctrlr_dom) { \
  GET_USER_DATA(ckey)      \
  key_user_data *user_data = \
                reinterpret_cast<key_user_data *>(ckey->get_user_data()); \
  if (((ctrlr_dom).ctrlr) && \
                  strlen(reinterpret_cast<char *>((ctrlr_dom).ctrlr)))  {\
    uuu::upll_strncpy(user_data->ctrlr_id, (ctrlr_dom).ctrlr, \
                                         (kMaxLenCtrlrId+1)); \
  } else { \
     (ctrlr_dom).ctrlr = NULL; \
  }\
  if (((ctrlr_dom).domain) && \
                 strlen(reinterpret_cast<char *>((ctrlr_dom).domain)))  \
    uuu::upll_strncpy(user_data->domain_id, ((ctrlr_dom).domain), \
                                                   (kMaxLenDomainId+1)); \
  else  \
    (ctrlr_dom).domain = NULL; \
}

#define GET_USER_DATA_FLAGS(ckey, rename) \
{ \
  key_user_data_t *user_data = \
                reinterpret_cast<key_user_data_t *>((ckey)->get_user_data()); \
  if (user_data) { \
    rename = user_data->flags; \
  } \
}

#define GET_USER_DATA_CTRLR(ckey, ctrlr) \
{ \
  key_user_data_t *user_data = \
               reinterpret_cast<key_user_data_t *>(ckey->get_user_data()); \
  if (user_data) { \
    (ctrlr) = \
       (reinterpret_cast<key_user_data_t *>(ckey->get_user_data()))->ctrlr_id; \
  } \
}

#define GET_USER_DATA_DOMAIN(ckey, domain) \
{ \
  key_user_data_t *user_data = \
                 reinterpret_cast<key_user_data_t *>(ckey->get_user_data()); \
  if (user_data) { \
    (domain) = \
      (reinterpret_cast<key_user_data_t *>(ckey->get_user_data()))->domain_id; \
  } \
}

#define GET_USER_DATA_CTRLR_DOMAIN(ckey, ctrlr_domain) \
{ \
  key_user_data_t *user_data = \
                reinterpret_cast<key_user_data_t *>(ckey->get_user_data()); \
  if (user_data) { \
    if (strlen(reinterpret_cast<char *>((reinterpret_cast<key_user_data_t *> \
                                     (ckey->get_user_data()))->ctrlr_id))) { \
    (ctrlr_domain).ctrlr  = \
       (reinterpret_cast<key_user_data_t *>(ckey->get_user_data()))->ctrlr_id; \
    } \
    if (strlen(reinterpret_cast<char *>((reinterpret_cast<key_user_data_t *> \
                                    (ckey->get_user_data()))->domain_id))) { \
    (ctrlr_domain).domain  = \
       (reinterpret_cast<key_user_data_t *>(ckey->get_user_data()))->domain_id;\
    } \
  } \
}

#define SET_USER_DATA_FLAGS(ckey, rename) \
{ \
  GET_USER_DATA(ckey)    \
  key_user_data_t  *user_data = \
               reinterpret_cast<key_user_data_t *>(ckey->get_user_data()); \
  user_data->flags = (rename); \
}

#define FREE_IF_NOT_NULL(key) \
do { \
  if (key) \
    free(key); \
} while (0);

#define DELETE_IF_NOT_NULL(key) \
do { \
  if (key)\
  delete (key);\
  (key) = NULL;\
} while (0);

#define POM_UPDATE_KTS(keytype, update) \
{ \
  update = ((keytype == UNC_KT_FLOWLIST) \
           || (keytype == UNC_KT_POLICING_PROFILE_ENTRY) \
           || (keytype == UNC_KT_FLOWLIST_ENTRY) \
           || (keytype == UNC_KT_VTN_POLICINGMAP) \
           || (keytype == UNC_KT_VTN_FLOWFILTER_ENTRY))? true:false; \
}

#define GET_TABLE_TYPE(keytype, tbl) \
{ \
  tbl = ((keytype == UNC_KT_VTN) || (keytype == UNC_KT_POLICING_PROFILE) \
         ||(keytype == UNC_KT_POLICING_PROFILE_ENTRY) \
         || (keytype == UNC_KT_FLOWLIST) || (keytype == UNC_KT_FLOWLIST_ENTRY) \
         || (keytype == UNC_KT_VTN_POLICINGMAP) \
         || (keytype == UNC_KT_VTN_FLOWFILTER) \
         || (keytype == UNC_KT_VTN_FLOWFILTER_ENTRY))? CTRLRTBL:MAINTBL; \
}

#define IS_GLOBAL_KEYTYPE(keytype, flag) \
{ \
  flag = ((keytype == UNC_KT_VTN) || (keytype == UNC_KT_POLICING_PROFILE) \
         ||(keytype == UNC_KT_POLICING_PROFILE_ENTRY) \
         || (keytype == UNC_KT_FLOWLIST) || (keytype == UNC_KT_FLOWLIST_ENTRY) \
         || (keytype == UNC_KT_VTN_POLICINGMAP) \
         || (keytype == UNC_KT_VTN_FLOWFILTER) \
         || (keytype == UNC_KT_VTN_FLOWFILTER_ENTRY))? true:false; \
}
#if 0
bool IsGlobalKT(unc_key_type_t keytype) {
  return ((keytype == UNC_KT_VTN) || (keytype == UNC_KT_POLICING_PROFILE)
          ||(keytype == UNC_KT_POLICING_PROFILE_ENTRY)
          || (keytype == UNC_KT_FLOWLIST) || (keytype == UNC_KT_FLOWLIST_ENTRY)
          || (keytype == UNC_KT_VTN_POLICINGMAP)
          || (keytype == UNC_KT_VTN_FLOWFILTER)
          || (keytype == UNC_KT_VTN_FLOWFILTER_ENTRY));
}
#endif
#define READ_OP(op) ((op == UNC_OP_READ) || (op == UNC_OP_READ_SIBLING) || \
                     (op == UNC_OP_READ_SIBLING_BEGIN) || \
                     (op == UNC_OP_READ_SIBLING_COUNT) || \
                     (op == UNC_OP_READ_BULK) || (op == UNC_OP_READ_NEXT))

#define KEYTYPE_WITHOUT_DOMAIN(keytype, domain) \
{ \
  domain = ((keytype == UNC_KT_POLICING_PROFILE) \
         || (keytype == UNC_KT_POLICING_PROFILE_ENTRY) \
         || (keytype == UNC_KT_FLOWLIST) \
         || (keytype == UNC_KT_FLOWLIST_ENTRY) \
         || (keytype == UNC_KT_VTEP_GRP) \
         || (keytype == UNC_KT_VTEP_GRP_MEMBER))?true:false; \
}

#define IS_POM_KT(keytype, flag) \
{ \
  flag = ((keytype == UNC_KT_VTN_POLICINGMAP) \
         || (keytype == UNC_KT_VBR_POLICINGMAP) \
         || (keytype == UNC_KT_VBRIF_POLICINGMAP) \
         || (keytype == UNC_KT_VTN_FLOWFILTER) \
         || (keytype == UNC_KT_VTN_FLOWFILTER_ENTRY) \
         || (keytype == UNC_KT_VBR_FLOWFILTER) \
         || (keytype == UNC_KT_VBR_FLOWFILTER_ENTRY) \
         || (keytype == UNC_KT_VBRIF_FLOWFILTER) \
         || (keytype == UNC_KT_VBRIF_FLOWFILTER_ENTRY) \
         || (keytype == UNC_KT_VRTIF_FLOWFILTER) \
         || (keytype == UNC_KT_VRTIF_FLOWFILTER_ENTRY) \
         || (keytype == UNC_KT_VTERMIF_POLICINGMAP) \
         || (keytype == UNC_KT_VTERMIF_FLOWFILTER) \
         || (keytype == UNC_KT_VTERMIF_FLOWFILTER_ENTRY) \
         || (keytype == UNC_KT_POLICING_PROFILE) \
         || (keytype == UNC_KT_FLOWLIST) \
         || (keytype == UNC_KT_POLICING_PROFILE_ENTRY))?true:false; \
}

#define IS_INTERFACE_KEY(kt_type) ((kt_type == UNC_KT_VBR_IF) || \
                                   (kt_type == UNC_KT_VRT_IF) || \
                                   (kt_type == UNC_KT_VTEP_IF) || \
                                   (kt_type == UNC_KT_VTUNNEL_IF) || \
                                   (kt_type == UNC_KT_VUNK_IF) || \
                                   (kt_type == UNC_KT_VTERM_IF))

#define IS_REDIRECT_INTERFACE_KEY(kt_type) (((kt_type) == UNC_KT_VBR_IF) || \
                                            ((kt_type) == UNC_KT_VRT_IF) || \
                                            ((kt_type) == UNC_KT_VTERM_IF))

#define ISRENAME_KEYTYPE(keytype) ((UNC_KT_VTN == keytype) || \
        (UNC_KT_FLOWLIST == keytype) || \
        (UNC_KT_POLICING_PROFILE == keytype) || (UNC_KT_VBRIDGE == keytype) || \
        (UNC_KT_VROUTER == keytype) || (UNC_KT_VLINK == keytype)  || \
        (UNC_KT_VTERMINAL == keytype)) \



#define IS_POM_IF_KT(keytype) (((keytype) == UNC_KT_VBRIF_POLICINGMAP) \
                            || ((keytype) == UNC_KT_VBRIF_FLOWFILTER) \
                            || ((keytype) == UNC_KT_VBRIF_FLOWFILTER_ENTRY) \
                            || ((keytype) == UNC_KT_VRTIF_FLOWFILTER) \
                            || ((keytype) == UNC_KT_VRTIF_FLOWFILTER_ENTRY) \
                            || ((keytype) == UNC_KT_VTERMIF_POLICINGMAP) \
                            || ((keytype) == UNC_KT_VTERMIF_FLOWFILTER) \
                            || ((keytype) == UNC_KT_VTERMIF_FLOWFILTER_ENTRY))


#define IS_GLOBAL_KEY_TYPE(keytype) ((UNC_KT_VTN == keytype) || \
                                     (UNC_KT_FLOWLIST == keytype) || \
                                     (UNC_KT_POLICING_PROFILE == keytype))

enum state_notification {
  kCommit,
  kPortUnknown,
  kPortUnknownFromDown,
  kPortFault,
  kPortFaultFromUnknown,
  kPortFaultReset,
  kPortFaultResetFromUnknown,
  kPathFault,
  kPathFaultReset,
  kCtrlrDisconnect,
  kReConnect,
  kPortUp,
  kVtnExhaustion,
  kVtnExhaustionReset
};
enum flag_status {
  PORT_UP = 0x00,
  PORT_UNKNOWN = 0x01,
  PORT_FAULT = 0x02,
  ADMIN_DISABLE = 0x04,
  REMOTE_DOWN = 0x08,
  PATH_FAULT = 0x10,
  REMOTE_PATH_FAULT = 0x20,
  REMOTE_VTN_EXHAUSTION = 0x40,
};

#define POM_RENAME_KT(ktype) (((ktype) == UNC_KT_VBR_POLICINGMAP) || \
                              ((ktype) == UNC_KT_VBRIF_POLICINGMAP) || \
                              ((ktype) == UNC_KT_VBR_FLOWFILTER_ENTRY) || \
                              ((ktype) == UNC_KT_VBRIF_FLOWFILTER_ENTRY) || \
                              ((ktype) == UNC_KT_VRTIF_FLOWFILTER_ENTRY))
#define RENAME 0x07
#define RENAME_BITS 0x03
#define GET_RENAME_FLAG(rename, ktype) \
{ \
  if (POM_RENAME_KT(ktype)) \
    rename = rename & RENAME; \
  else\
    rename = rename & RENAME_BITS; \
}

#define NO_VTN_RENAME 0xFE
#define NO_VN_RENAME 0xFD
#define NO_PP_RENAME 0x00
#define NO_PM_RENAME 0x03
#define NO_FL_RENAME 0x00
#define NO_FF_RENAME 0x03

#define VTN_RENAME 0x01
#define VN_RENAME 0x02
#define PP_RENAME 0x01
#define PPE_RENAME 0x02
#define PM_RENAME 0x04
#define FL_RENAME 0x01
#define FF_RENAME 0x04
#define NO_RENAME 0x00

enum val_rename_vnode_index {
  UPLL_CTRLR_VTN_NAME_VALID = 0,
  UPLL_CTRLR_VNODE_NAME_VALID
};

typedef struct key_vnode {
  key_vtn vtn_key;
  uint8_t vnode_name[kMaxLenVnodeName+1];
} key_vnode_t;

typedef struct key_vnode_if_t {
  key_vnode_t vnode_key;
  uint8_t vnode_if_name[kMaxLenInterfaceName+1];
} key_vnode_if;

enum rename_key {
  UNC_RENAME_KEY,
  CTRLR_RENAME_KEY
};

#define VIF_TYPE 0xF0
#define VIF_TYPE_BOUNDARY 0xC0
#define VIF_TYPE_LINKED 0x30

enum vnode_if_type {
  kInvalid = 0x0,
  kVbrIf,
  kVrtIf,
  kVunkIf,
  kVtepIf,
  kVtunnelIf,
  /* VlanmapOnBoundary: kVlanMap is used when boundary
   * vnode_if is mapped to SD or SW */
  kVlanMap,
  //  kVbrPortMap is used when boundary vnode_if type is unified vBridge
  kVbrPortMap
};

#define VLINK_FLAG_NODE_TYPE 0xFC
#define VLINK_FLAG_NODE1_TYPE 0xE0
#define VLINK_FLAG_NODE2_TYPE 0x1C
#define kVlinkVnodeIf2Type  2
#define kVlinkVnodeIf1Type  5

#define GET_VLINK_NODE1_TYPE(vlink_flag) \
  ((vlink_flag  & VLINK_FLAG_NODE1_TYPE) >> kVlinkVnodeIf1Type)

#define GET_VLINK_NODE2_TYPE(vlink_flag) \
  ((vlink_flag  & VLINK_FLAG_NODE2_TYPE) >> kVlinkVnodeIf2Type)

enum if_type {
  kUnboundInterface = 0x0,
  kMappedInterface,
  kBoundaryInterface,
  kLinkedInterface
};

enum vn_if_type {
  kUnlinkedInterface  = 0x0,
  kVlinkBoundaryNode1 = 0x80,
  kVlinkBoundaryNode2 = 0x40,
  kVlinkInternalNode1 = 0x20,
  kVlinkInternalNode2 = 0x10
};

enum NotifyPOMForPortMapInfo {
  kPortMapNoChange = 0,
  kPortMapCreated,  // Used to notify POM when PortMap is Created
  kPortMapDeleted,  // Used to notifyPOM when PortMap is Deleted
  kPortMapUpdated,  // PortMap is Updated. No Notification is required for POM
};

enum InterfacePortMapInfo {
  kVlinkPortMapNotConfigured = 0x00,
  kVlinkConfigured = 0x01,
  kPortMapConfigured = 0x02, /*Vlinked*/
  kVlinkPortMapConfigured = 0x03     /* Boundart*/
};

typedef struct val_rename_vnode {
  uint8_t valid[2];
  uint8_t ctrlr_vtn_name[kMaxLenVtnName+1];
  uint8_t ctrlr_vnode_name[kMaxLenVnodeName+1];
} val_rename_vnode_t;


struct vlink_compare {
  inline bool operator()(const key_vlink_t &keyvlink1,
                          const key_vlink_t keyvlink2) const {
    int ret = strcmp((const char *)keyvlink1.vtn_key.vtn_name,
                     (const char *)keyvlink2.vtn_key.vtn_name);
    if (ret == 0) {
      return (strcmp((const char *)keyvlink1.vlink_name,
                     (const char*)keyvlink2.vlink_name) < 0);
    } else {
      return (ret < 0);
    }
  }
};


enum VlinkNodePosition {
  kVlinkVnode1 = 0x80,
  kVlinkVnode2 = 0x40
};

#define VLINK_FLAG_NODE_POS 0xC0
/* VlanmapOnBoundary: Below flags used to set/clear
 * user or boundary configured bit in vlanmap flag */
#define USER_VLANMAP_FLAG 0x40
#define BOUNDARY_VLANMAP_FLAG 0x80
#define BOUNDARY_UVBRIF_FLAG 0x80
#define USER_VBRPM_IF_FLAG 0x40

// new vBridge flags
#define USER_VBRPORTMAP_FLAG 0x40
#define BOUNDARY_VBRPORTMAP_FLAG 0x80

typedef struct key_user_data {
  uint8_t                 ctrlr_id[kMaxLenCtrlrId+1];
  uint8_t                 domain_id[kMaxLenDomainId+1];
  uint8_t                 flags;
} key_user_data_t;

enum KUpllReadOp {
  kOpNotRead = 0x00,
  kOpReadSingle = 0x01,
  kOpReadMultiple = 0x02,
  kOpReadExist = 0x04,
  kOpReadCount = 0x08,
  kOpReadDiff = 0x10,
  kOpReadDiffUpd = 0x20
};

enum KUpllMatchOp {
  kOpMatchNone = 0x0,
  kOpMatchCtrlr = 0x01,
  kOpMatchDomain = 0x02,
  kOpMatchFlag = 0x04,
  kOpMatchCs = 0x08
};

enum KUpllInOutOp {
  kOpInOutNone  =  0x00,
  kOpInOutCtrlr = 0x01,
  kOpInOutDomain = 0x02,
  kOpInOutFlag = 0x04,
  kOpInOutCs = 0x08
};

struct DbSubOp {
  unsigned int readop;
  unsigned int matchop;
  unsigned int inoutop;
};

enum MoMgrTables {
  MAINTBL = 0,
  RENAMETBL,
  CTRLRTBL,
  CONVERTTBL,
  VBIDTBL,
  GVTNIDTBL,
  MAX_MOMGR_TBLS
};

enum AdaptType {
  ADAPT_ALL = 0,
  ADAPT_ONE
};

enum BindStructTypes {
  CFG_KEY,
  CFG_VAL,
  CFG_META_VAL,
  ST_VAL,
  ST_META_VAL,
  CK_VAL,
  CK_VAL2,
  CS_VAL,
  CFG_DEF_VAL,
  CFG_ST_VAL,
  CFG_ST_META_VAL,
  CFG_INPUT_KEY,
  CFG_MATCH_KEY
};


struct BindInfo {
  uint64_t   index;
  BindStructTypes struct_type;
  size_t offset;
  uud::DalCDataType app_data_type;
  uint16_t     array_size;
};


typedef struct key_rename_vnode_info {
  uint8_t new_unc_vtn_name[kMaxLenVtnName+1];
  uint8_t old_unc_vtn_name[kMaxLenVtnName+1];
  uint8_t new_unc_vnode_name[kMaxLenVnodeName+1];
  uint8_t old_unc_vnode_name[kMaxLenVnodeName+1];
  uint8_t ctrlr_vtn_name[kMaxLenVtnName+1];
  uint8_t ctrlr_vnode_name[kMaxLenVnodeName+1];
  uint8_t old_policingprofile_name[kMaxLenPolicingProfileName+1];
  uint8_t new_policingprofile_name[kMaxLenPolicingProfileName+1];
  uint8_t ctrlr_profile_name[kMaxLenPolicingProfileName+1];
  uint8_t old_flowlist_name[kMaxLenFlowListName+1];
  uint8_t new_flowlist_name[kMaxLenFlowListName+1];
  uint8_t ctrlr_flowlist_name[kMaxLenFlowListName+1];
}key_rename_vnode_info_t;





class Table {
  uudst::kDalTableIndex     tbl_index;
  unc_key_type_t     key_type;
  uui::IpctSt::IpcStructNum   key_struct;
  IpctSt::IpcStructNum   val_struct;
  BindInfo           *db_info;
  int                 nattr;

 public:
  Table(uudst::kDalTableIndex itbl_index, unc_key_type_t ikey_type,
         BindInfo *binfo, uui::IpctSt::IpcStructNum ikey_struct,
         IpctSt::IpcStructNum ival_struct,
        int inattr):
      key_type(ikey_type), key_struct(ikey_struct),
      val_struct(ival_struct), nattr(inattr) {
        tbl_index = itbl_index;
        db_info = binfo;
  }
  ~Table() {
  }
  unc_key_type_t get_key_type() { return key_type; }
  uudst::kDalTableIndex GetTblIndex() { return tbl_index; }
  bool GetBindInfo(BindInfo *&binfo,
                   int &nfields) {
    binfo = db_info;
    nfields = nattr;
    return true;
  }
  void GetKeyValStruct(
             int &oktype, int &okstruct, int &ovstruct) {
    oktype = key_type;
    okstruct = key_struct;
    ovstruct = val_struct;
  }
};


class MoMgrImpl : public MoManager {
 private:
  upll_rc_t CreateImportMoImpl(IpcReqRespHeader *req,
                           ConfigKeyVal *ikey,
                           DalDmlIntf *dmi,
                           const char *ctrlr_id,
                           const char *domain_id,
                           upll_import_type import_type);

  // Returns Query String for Read Import based on operation and keytype
  std::string GetReadImportQueryString(unc_keytype_operation_t op,
                                       unc_key_type_t kt) const;

  bool OperStatusSupported(unc_key_type_t kt) {
    switch (kt) {
      case UNC_KT_VTN:
      case UNC_KT_VBRIDGE:
      case UNC_KT_VROUTER:
      case UNC_KT_VBR_IF:
      case UNC_KT_VRT_IF:
      case UNC_KT_VBR_VLANMAP:
      case UNC_KT_VTERMINAL:
      case UNC_KT_VTERM_IF:
      case UNC_KT_VBR_PORTMAP:
        return true;
      default:
        return false;
    }
  }

  // Converts the driver returned err_ckv specific to VTN
  // This method is used when the err_ckv sent from driver
  // does not have all required key values filled
  // Calls GetRenamedUncKey as well
  upll_rc_t AdaptErrValToVtn(ConfigKeyVal *ckv_drv_rslt,
                             DalDmlIntf *dmi,
                             uint8_t* ctrlr_id,
                             upll_keytype_datatype_t dt_type);

  /* @brief         Convert vlink keytype error to corresponding vbrif keytype
   *                if the given vlink is part of vbrif portmap in PFC.
   *
   * @param[in]  err_ckv      Pointer to the ConfigKeyVal Structure
   * @param[in]  datatype     Specifies the configuration CANDIDATE/RUNNING
   * @param[in]  keytype      keytype
   * @param[in]  dmi          Pointer to the DalDmlIntf(DB Interface)
   *
   * @retval  UPLL_RC_SUCCESS                 Translation required.
   * @retval  UPLL_RC_ERR_GENERIC             Generic failure.
   * @retval  UPLL_RC_ERR_DB_ACCESS           DB Read/Write error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE    Translation not required.
   **/
  upll_rc_t TranslateVlinkTOVbrIfError(
      ConfigKeyVal *err_ckv,
      DalDmlIntf *dmi,
      upll_keytype_datatype_t datatype);

  /* @brief      Check rename operation is possible or not.
   *
   * @param[in]  ikey        Pointer to the ConfigKeyVal Structure
   * @param[in]  ctrlr_name  Controller name
   * @param[in]  dmi         Pointer to the DalDmlIntf(DB Interface)
   *
   * @retval  UPLL_RC_SUCCESS                       Translation required.
   * @retval  UPLL_RC_ERR_GENERIC                   Generic failure.
   * @retval  UPLL_RC_ERR_DB_ACCESS                 DB Read/Write error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE          Rename allowed.
   * @retval  UPLL_RC_ERR_NOT_ALLOWED_AT_THIS_TIME  Rename not allowed for the
   *                                                existing configuration
   **/
  upll_rc_t ValidateRename(ConfigKeyVal *ikey,
                           DalDmlIntf *dmi,
                           uint8_t *ctrlr_name);

  bool ThresholdAlarmProcessingRequired(ConfigKeyVal *can_key,
                                        ConfigKeyVal *run_key,
                                        unc_keytype_operation_t op,
                                        TcConfigMode config_mode);

  upll_rc_t ProcessSpineDomainAndLabel(ConfigKeyVal *can_key,
                                       ConfigKeyVal *run_key,
                                       ConfigKeyVal *upd_key,
                                       unc_keytype_operation_t op,
                                       TcConfigMode config_mode,
                                       DalDmlIntf* dmi);

 protected:
  Table **table;
  int ntable;
  unc_key_type_t *child;
  int nchild;
  upll_rc_t DalToUpllResCode(DalResultCode result_code);

 /* @brief    Populate val_vtn_neighbor for the READ/READ_SIBLING operations
  *
  * @param[in/out] key   Pointer to the ConfigKeyVal Structure
  * @param[in]     dmi    Pointer to the DalDmlIntf(DB Interface)
  *
  * @retval  UPLL_RC_SUCCESS                    Completed successfully.
  * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
  * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
  * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
  * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
  *
  **/
  virtual upll_rc_t PopulateValVtnNeighbor(ConfigKeyVal *&key,
                                   DalDmlIntf *dmi) {
    return UPLL_RC_SUCCESS;
  }

  upll_rc_t TxCopyRenameTableFromCandidateToRunning(
                                    unc_key_type_t key_type,
                                    unc_keytype_operation_t op,
                                    DalDmlIntf* dmi,
                                    TcConfigMode config_mode,
                                    std::string vtn_name);
  upll_rc_t TxComputeOperStatusandCommit(
                         unc_key_type_t key_type,
                         CtrlrCommitStatusList *ctrlr_commit_status,
                         unc_keytype_operation_t op,
                         DalDmlIntf* dmi, TcConfigMode config_mode,
                         std::string vtn_name);
  upll_rc_t UpdateGWPortStatus(ConfigKeyVal *ckey, DalDmlIntf *dmi,
                              uint32_t driver_result);
  upll_rc_t CheckUnifiedOperStatus(ConfigKeyVal *ikey,
                                            DalDmlIntf *dmi);


#if 0
  /**
    * @brief Get configkey val with oper status initialized to UNINIT
    *
    * @param[out]  ck_vn  pointer to ConfigKeyVal
    * @param[in]   ktype  key type
    * @param[in]   dmi    Poiner to database connection params.
    *
    * @retval UPLL_RC_SUCCESS      Successful
    * @retval UPLL_RC_ERR_GENERIC  Generic error
    */
  template<typename T1, typename T2>
  upll_rc_t GetCkvWithOperSt(ConfigKeyVal *&ck_vn,
                               unc_key_type_t ktype,
                               DalDmlIntf     *dmi);
#else
    /* @brief      Gets ports with uninitialized oper status
     *             - ports whose status have to be obtained from physical
     *
     * @param[out]  ikey     Pointer to a list of configkeyvals
     * @param[in]   dmi      Database connection parameter
     *
     * @retval  UPLL_RC_SUCCESS      Completed successfully.
     * @retval  UPLL_RC_ERR_GENERIC  Generic failure.
     *
     **/
    upll_rc_t GetUninitOperState(ConfigKeyVal *&ck_vn, DalDmlIntf *dmi);
#endif

  virtual upll_rc_t SwapKeyVal(ConfigKeyVal *ikey,
                               ConfigKeyVal *&okey,
                               DalDmlIntf *dmi,
                               uint8_t *ctrlr,
                               bool &no_rename) {
    return UPLL_RC_ERR_GENERIC;
  };
  upll_rc_t ValidateDeleteMoReq(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi);
  upll_rc_t DeleteCandidateMo(IpcReqRespHeader *req,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi);

  virtual upll_rc_t CreateAuditMoImpl(ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi,
                                      const char *ctrlr_id);

  virtual upll_rc_t CreatePIForVtnPom(IpcReqRespHeader *req,
                                      ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi,
                                      const char *ctrlr_id) {
    return UPLL_RC_SUCCESS;
  }

  /**
   * @brief  Update parent oper status on delete for Transaction commit
   *
   * @param[int]  ikey          ConfigKeyVal instance
   * @param[in]   dmi           Database connection parameter

   * @retval  UPLL_RC_SUCCESS      Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC  Generic failure.
   */
  virtual upll_rc_t UpdateParentOperStatus(ConfigKeyVal *ikey,
                                           DalDmlIntf *dmi,
                                           uint32_t driver_result) {
    UPLL_LOG_DEBUG("Unsupported operation for this keytype %d\n",
                      (ikey)?ikey->get_key_type():0);
    return UPLL_RC_ERR_GENERIC;
  }

  /**
   * @brief  Update config status for Transaction commit
   *
   * @param[in/out]  req          ConfigKeyVal instance
   * @param[in]      op           Operation(CREATE, DELETE or UPDATE)
   * @param[in]      result       UNC_CS_APPLIED or UNC_CS_NOT_APPLIED
   *
   **/
  virtual upll_rc_t UpdateConfigStatus(ConfigKeyVal *req,
                                unc_keytype_operation_t op,
                                uint32_t result,
                                ConfigKeyVal *upd_key = NULL,
                                DalDmlIntf *dmi = NULL,
                                ConfigKeyVal *ctrlr_key = NULL) = 0;
  /**
   * @brief  Update config status for commit result and vote result.
   *
   * @param[in/out]  ckv_running  ConfigKeyVal instance.
   * @param[in]      cs_status    either UNC_CS_INVALID or UNC_CS_APPLIED.
   * @param[in]      phase        specify the phase (CREATE, DELETE or UPDATE)
   * @param[in]      dmi          Pointer to the DalDmlIntf(DB Interface)
   *
   **/
  virtual upll_rc_t UpdateAuditConfigStatus(
                                     unc_keytype_configstatus_t cs_status,
                                     uuc::UpdateCtrlrPhase phase,
                                     ConfigKeyVal *&ckv_running,
                                     DalDmlIntf *dmi) = 0;

  virtual upll_rc_t UpdateCtrlrConfigStatus(
                                     unc_keytype_configstatus_t cs_status,
                                     uuc::UpdateCtrlrPhase phase,
                                     ConfigKeyVal *&ckv_running) {
    return UPLL_RC_SUCCESS;
  }
  virtual upll_rc_t UpdateRenameKey(ConfigKeyVal *&ikey,
                                    upll_keytype_datatype_t dt_type,
                                    unc_keytype_operation_t op,
                                    DalDmlIntf *dmi,
                                    DbSubOp *pdbop,
                                    MoMgrTables tbl);

  virtual upll_rc_t GetDiffRecord(ConfigKeyVal *ckv_running,
                                   ConfigKeyVal *ckv_audit,
                                   uuc::UpdateCtrlrPhase phase, MoMgrTables tbl,
                                   ConfigKeyVal *&ckv_driver_req,
                                   DalDmlIntf *dmi,
                                   bool &invalid_attr,
                                   bool check_audit_phase);
                       /* chech_audit_phase will be true only on audit phase */

  virtual upll_rc_t  ValidateMessage(IpcReqRespHeader *req,
                                     ConfigKeyVal *ikey) = 0;

  /**
   * @brief  Gets the valid array position of the variable in the value
   *         structure from the table in the specified configuration
   *
   * @param[in]     val      pointer to the value structure
   * @param[in]     indx     database index for the variable
   * @param[out]    valid    position of the variable in the valid array -
   *                          NULL if valid does not exist.
   * @param[in]     dt_type  specifies the configuration
   * @param[in]     tbl      specifies the table containing the given value
   *
   **/
  virtual upll_rc_t GetValid(void*val,
                             uint64_t indx,
                             uint8_t *&valid,
                             upll_keytype_datatype_t dt_type,
                             MoMgrTables tbl = MAINTBL)= 0;
  virtual upll_rc_t CreateCandidateMo(IpcReqRespHeader *req,
                                      ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi);

  upll_rc_t BindStartup(DalBindInfo *db_info,
                     upll_keytype_datatype_t dt_type,
                     MoMgrTables tbl = MAINTBL);

  upll_rc_t BindKeyAndVal(DalBindInfo *db_info,
                     upll_keytype_datatype_t dt_type,
                     MoMgrTables tbl = MAINTBL,
                     const uudst::kDalTableIndex index = uudst::kDalNumTables);

  upll_rc_t BindKeyAndValForMerge(DalBindInfo *db_info,
                     upll_keytype_datatype_t dt_type,
                     MoMgrTables tbl,
                     const uudst::kDalTableIndex index);

  virtual upll_rc_t BindAttr(DalBindInfo *db_info,
                             ConfigKeyVal *&req,
                             unc_keytype_operation_t op,
                             upll_keytype_datatype_t dt_type,
                             DbSubOp dbop, MoMgrTables tbl = MAINTBL);
  virtual upll_rc_t BindAttrRename(DalBindInfo *db_info, ConfigKeyVal *&req,
                                   unc_keytype_operation_t op,
                                   upll_keytype_datatype_t dt_type,
                                   DbSubOp dbop, MoMgrTables tbl = MAINTBL);

  /*const */uudst::kDalTableIndex GetTable(MoMgrTables tbl,
                                    upll_keytype_datatype_t dt_type) {
    if (NULL != table[tbl])
      return table[tbl]->GetTblIndex();
    else
      return uudst::kDalNumTables;
  }
  unc_key_type_t GetMoMgrKeyType(MoMgrTables tbl,
         upll_keytype_datatype_t dt_type) {
    if (table[tbl])
      return table[tbl]->get_key_type();
    else
      return UNC_KT_ROOT;
  }
  bool GetBindInfo(MoMgrTables tbl, upll_keytype_datatype_t dt_type,
                   BindInfo *&binfo, int &oattr) {
    if ((!table[tbl]) || (tbl >= ntable))
      return false;
    table[tbl]->GetBindInfo(binfo, oattr);
    return true;
  }
  /* @brief     To convert the value structure read from DB to
   *            VTNService during READ operations
   *
   * @param[in/out] ikey      Pointer to the ConfigKeyVal Structure
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   *
   **/
  virtual upll_rc_t AdaptValToVtnService(ConfigKeyVal *ikey,
                                         AdaptType adapt_type = ADAPT_ALL) {
    return UPLL_RC_SUCCESS;
  }
  /**
   * @brief      Method to assign ctrlr_dom
   *
   * @param[in/out]  ikey         ConfigVal pointer to input unc key
   * @param[in]      dmi          specifies the db connection info
   * @param[in]      ctrlr_dom    specifies the pointer to the controller id
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   */
  upll_rc_t GetUserDataCtrlrDomain(ConfigKeyVal *ikey,
                                   controller_domain *ctrlr_dom);
  /**
   * @brief     Perform semantic validation and corresponding
   *            vexternal/vexternalif conversion on key type specific,
   *            before sending to driver.
   *            This function can be invoked only during COMMIT/AUDIT phase.
   *
   * @param[in]  ck_new                   Contains candidate information (cannot
   *                                      be NULL)
   * @param[in]  ck_old                   Contains running information (It can
   *                                      be NULL during audit phase)
   * @param[in]  op                       Operation name.
   * @param[in]  dt_type                  Specifies the configuration
   *                                      CANDIDATE/RUNNING
   * @param[in]  keytype                  Specifies the keytype
   * @param[in]  dmi                      Pointer to the DalDmlIntf
   *                                      (DB Interface)
   * @param[out] not_send_to_drv          Decides whether the configuration
   *                                      needs to be sent to controller or not
   * @param[in]  audit_update_phase       Specifies whether the phase is commit
   *                                      or audit,
   *                                      true - audit / false - commit
   *
   * @retval  UPLL_RC_SUCCESS             Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC         Generic failure.
   * @retval  UPLL_RC_ERR_CFG_SEMANTIC    Failure due to semantic validation.
   * @retval  UPLL_RC_ERR_DB_ACCESS       DB Read/Write error.
   *
   */
  virtual upll_rc_t AdaptValToDriver(ConfigKeyVal *ck_new,
      ConfigKeyVal *ck_old,
      unc_keytype_operation_t op,
      upll_keytype_datatype_t dt_type,
      unc_key_type_t keytype,
      DalDmlIntf *dmi,
      bool &not_send_to_drv,
      bool audit_update_phase) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_SUCCESS;
  }
  /**
   * @brief     During Commit failure, convert driver error
   *            configuration based of the keytype and set to err_ckv.
   *            This function can be invoked only during COMMIT phase.
   *
   * @param[in]  ck_unc             It contains UNC specific names.
   * @param[in]  ck_driver          It contains Controller specific name.
   * @param[in]  dt_type            Specifies the configuration
   *                                CANDIDATE/RUNNING
   * @param[in]  dmi                Pointer to the DalDmlIntf
   *                                (DB Interface)
   * @param[in]  err_ckv            Pointer to the ConfigKeyVal Structure
   * @param[in]  ipc_resp           Pointer contains driver response.
   *
   * @retval  UPLL_RC_SUCCESS               Translation required.
   * @retval  UPLL_RC_ERR_GENERIC           Generic failure.
   * @retval  UPLL_RC_ERR_DB_ACCESS         DB Read/Write error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE  Translation not required.
   *
   */
  virtual upll_rc_t TxUpdateErrorHandler(ConfigKeyVal *ckv_unc,
                                          ConfigKeyVal *ck_driver,
                                          DalDmlIntf *dmi,
                                          upll_keytype_datatype_t dt_type,
                                          ConfigKeyVal **err_ckv,
                                          IpcResponse *ipc_resp);

 public:
  MoMgrImpl() {
    table = NULL;
    ntable = 0;
    child = NULL;
    nchild = 0;
  }
  virtual ~MoMgrImpl() {}
   /**
   * @brief  Perform Semantic Check to check Different vbridges
   *          contain same switch-id and vlan-id
   *
   * @param[in]       ikey        ConfigKeyVal
   * @param[out]      upll_rc_t   UPLL_RC_ERR_CFG_SEMANTIC on error
   *                                UPLL_RC_SUCCESS on success
   **/
  virtual upll_rc_t ValidateAttribute(ConfigKeyVal *kval, DalDmlIntf *dmi,
                                      IpcReqRespHeader *req = NULL) =0;

  /* @brief      - allocates a configkeyval with oper status = UNINIT
   *
   * @param[out]  ck_vn    Pointer to a configkeyval
   * @param[in]   ikey     null if no key is to be copied
   * @param[in]   dmi      Database connection parameter
   *
   * @retval  UPLL_RC_SUCCESS      Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC  Generic failure.
   *
   **/
  upll_rc_t GetCkvUninit(ConfigKeyVal *&ck_vn, ConfigKeyVal *ikey,
                         DalDmlIntf *dmi,
                         val_oper_status oper_status = UPLL_OPER_STATUS_UNINIT,
                         MoMgrTables tbl = MAINTBL);
  virtual upll_rc_t  ValidateCapability(IpcReqRespHeader *req,
                                        ConfigKeyVal *ikey,
                                        const char *ctrlr_name = NULL) = 0;
  upll_rc_t GetInstanceCount(ConfigKeyVal *ikey, char *ctrlr_id,
                             upll_keytype_datatype_t dt_type,
                             uint32_t *count,
                             DalDmlIntf *dmi,
                             MoMgrTables tbl);

  /**
   * @brief  Duplicates the input configkeyval including the key and val.
   * based on the tbl specified.
   *
   * @param[in]  okey   Output Configkeyval - allocated within the function
   * @param[in]  req    Input ConfigKeyVal to be duplicated.
   * @param[in]  tbl    specifies if the val structure belongs to the main table
   *                    controller table or rename table.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   **/
  virtual upll_rc_t DupConfigKeyVal(ConfigKeyVal *&okey,
                                    ConfigKeyVal *&req,
                                    MoMgrTables tbl = MAINTBL) = 0;
  /**
   * @brief      Method to check if individual portions of a key are valid
   *
   * @param[in/out]  ikey   pointer to ConfigKeyVal referring to a UNC resource
   * @param[in]      index  db index associated with the variable
   *
   * @retval         true   input key is valid
   * @retval         false  input key is invalid.
   **/
  virtual bool IsValidKey(void *tkey,
                          uint64_t index,
                          MoMgrTables tbl = MAINTBL) = 0;
  /**
   * @brief  Filters the attributes which need not be sent to controller
   *
   * @param[in/out]  val1   first record value instance.
   * @param[in]      val2   second record value instance.
   * @param[in]      audit  Not used for VTN
   * @param[in]      op     Operation to be performed
   *
   **/
  virtual bool FilterAttributes(void *&val1,
                        void *val2,
                        bool audit_status,
                        unc_keytype_operation_t op) {
  if (op != UNC_OP_CREATE)
    return CompareValidValue(val1, val2, audit_status);
  return false;
  }
  /**
   * @brief  Compares the valid value between two database records.
   *      if both the values are same, update the valid flag for corresponding
   *      attribute as invalid in the first record.
   *
   * @param[in/out]  val1   first record value instance.
   * @param[in]      val2   second record value instance.
   * @param[in]      audit  if true, CompareValidValue called from audit process
   *
   **/
  virtual bool CompareValidValue(void *&val1,
                                 void *val2,
                                 bool audit) = 0;

  virtual upll_rc_t PopulateDriverDeleteCkv(ConfigKeyVal *&vnpCkv,
                                            DalDmlIntf *dmi,
                                            upll_keytype_datatype_t dt_type) {
  return UPLL_RC_SUCCESS; }

  /**
   * @brief      Method to get renamed controller key from unc key.
   * On success unc key contains the controller key if renamed.
   *
   * @param[in/out]  ikey          ConfigVal pointer to input unc key
   * @param[in]      dt_type       specifies the configuration type
   * @param[in]      dmi           specifies the db connection info
   * @param[in]      ctrlr_name    specifies the pointer to the controller id
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   */
  virtual upll_rc_t GetRenamedControllerKey(ConfigKeyVal *ikey,
                                       upll_keytype_datatype_t dt_type,
                                       DalDmlIntf *dmi,
                                       controller_domain *ctrlr_dom = NULL) {
  return UPLL_RC_SUCCESS;
  }
  /**
   * @brief      Method to get a configkeyval of a specified keytype from
   * an input configkeyval
   *
   * @param[in/out]  okey                 pointer to output ConfigKeyVal
   * @param[in]      parent_key           pointer to the configkeyval from
   * which the output configkey val is initialized.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   */
  virtual upll_rc_t GetChildConfigKey(ConfigKeyVal *&okey,
                                      ConfigKeyVal *parent_key) = 0;
  /**
   * @brief      Method to get renamed unc key from controller key.
   * On success controller key contains the renamed unc key.
   *
   * @param[in/out]  ctrlr_key     ConfigVal pointer to input ctrlr key
   * @param[in]      dt_type       specifies the configuration type
   * @param[in]      dmi           specifies the db connection info
   * @param[in]      ctrlr_id      specifies the pointer to the controller id
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   */
  virtual upll_rc_t GetRenamedUncKey(ConfigKeyVal *ctrlr_key,
                                     upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dmi,
                                     uint8_t *ctrlr_id) = 0;
  virtual upll_rc_t GetChildConvertConfigKey(ConfigKeyVal *&okey,
                                             ConfigKeyVal *parent_key) {
    return UPLL_RC_ERR_GENERIC;
  }
  virtual upll_rc_t GetVbIdChildConfigKey(ConfigKeyVal *&okey,
                                          ConfigKeyVal *parent_key) {
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t ReadConfigDB(ConfigKeyVal *ikey,
                         upll_keytype_datatype_t dt_type,
                         unc_keytype_operation_t op,
                         DbSubOp dbop ,
                         DalDmlIntf *dmi,
                         MoMgrTables tbl = MAINTBL);
  upll_rc_t ReadConfigDB(ConfigKeyVal *ikey,
                         upll_keytype_datatype_t dt_type,
                         unc_keytype_operation_t op,
                         DbSubOp dbop ,
                         uint32_t &sibling_count,
                         DalDmlIntf *dmi,
                         MoMgrTables tbl = MAINTBL);
  virtual upll_rc_t DeleteChildren(ConfigKeyVal *ikey,
                           ConfigKeyVal *pkey,
                           upll_keytype_datatype_t dt_type,
                           DalDmlIntf *dmi,
                           TcConfigMode config_mode,
                           string vtn_name,
                           MoMgrTables tbl = MAINTBL);
  /**
   * @brief      Method to determine if given key is renamed
   * based on input rename flag, val structure of i/p key is modified
   *
   * @param[in]      ikey     pointer to input ConfigKeyVal
   * @param[in]      dt_type  specifies the configuration type
   * @param[in]      dmi      specifies the db connection info
   * @param[in/out]  rename   if rename is 0 input key is duplicated before
   *                          reading from db.
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   */
  upll_rc_t IsRenamed(ConfigKeyVal *ikey,
                      upll_keytype_datatype_t dt_type,
                      DalDmlIntf *dmi,
                      uint8_t &rename);
  upll_rc_t UpdateConfigDB(ConfigKeyVal *ikey,
                           upll_keytype_datatype_t dt_type,
                           unc_keytype_operation_t op,
                           DalDmlIntf *dmi,
                           MoMgrTables tbl = MAINTBL);
  upll_rc_t UpdateConfigDB(ConfigKeyVal *ikey,
                           upll_keytype_datatype_t dt_type,
                           unc_keytype_operation_t op,
                           DalDmlIntf *dmi,
                           DbSubOp *pdbop,
                           MoMgrTables tbl = MAINTBL);
  upll_rc_t UpdateConfigDB(ConfigKeyVal *ikey,
                           upll_keytype_datatype_t dt_type,
                           unc_keytype_operation_t op,
                           DalDmlIntf *dmi,
                           TcConfigMode cfg_mode,
                           string vtn_name,
                           MoMgrTables tbl = MAINTBL);
  upll_rc_t UpdateConfigDB(ConfigKeyVal *ikey,
                           upll_keytype_datatype_t dt_type,
                           unc_keytype_operation_t op,
                           DalDmlIntf *dmi,
                           DbSubOp *pdbop,
                           TcConfigMode cfg_mode,
                           string vtn_name,
                           MoMgrTables tbl = MAINTBL);
  upll_rc_t DiffConfigDB(upll_keytype_datatype_t dt_cfg1,
                         upll_keytype_datatype_t dt_cfg2,
                         unc_keytype_operation_t op,
                         ConfigKeyVal *&req,
                         ConfigKeyVal *&nreq,
                         DalCursor **cfg1_cursor,
                         DalDmlIntf *dmi,
                         TcConfigMode config_mode,
                         std::string vtn_name,
                         MoMgrTables tbl = MAINTBL);
  upll_rc_t DiffConfigDB(upll_keytype_datatype_t dt_cfg1,
                         upll_keytype_datatype_t dt_cfg2,
                         unc_keytype_operation_t op,
                         ConfigKeyVal *&req,
                         ConfigKeyVal *&nreq,
                         DalCursor **cfg1_cursor,
                         DalDmlIntf *dmi,
                         uint8_t *cntrlr_id,
                         TcConfigMode config_mode,
                         std::string vtn_name,
                         MoMgrTables tbl = MAINTBL,
                         bool read_withcs = false,
                         bool auditdiff_with_flag = false);
  /**
   * @brief To populate the Ipc command
   *
   */
  upll_rc_t SendIpcReq(uint32_t session_id,
                      uint32_t config_id,
                      unc_keytype_operation_t op,
                      upll_keytype_datatype_t dt_type,
                      ConfigKeyVal *&ckv,
                      controller_domain_t *ctrlr_dom,
                      IpcResponse *ipc_resp);
  virtual upll_rc_t EnqueCfgNotification(unc_keytype_operation_t op,
                                         upll_keytype_datatype_t dt_type,
                                         ConfigKeyVal *ctrlr_key);
  /* implementation of pure virtual functions from MoMgr */
  virtual upll_rc_t CreateMo(IpcReqRespHeader *req,
                             ConfigKeyVal *ikey,
                             DalDmlIntf *dmi);
  virtual upll_rc_t CreateImportMo(IpcReqRespHeader *req,
                                   ConfigKeyVal *key,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id,
                                   const char *domain_id,
                                   upll_import_type import_type);

  /**
   * @brief      Method used to Delete the Values in the specified key type.
   *
   * @param[in]  req         contains first 8 fields of input request structure
   * @param[in]  ikey        key and value structure
   * @param[in]  dmi         Pointer to DalDmlIntf Class.
   *
   * @retval     UPLL_RC_SUCCESS            Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC        Failure case.
   */


  virtual upll_rc_t DeleteMo(IpcReqRespHeader *req,
                             ConfigKeyVal *ikey,
                             DalDmlIntf *dmi);

  /**
   * @brief      Method used to Update the Values in the specified key type.
   *
   * @param[in]  req         contains first 8 fields of input request structure
   * @param[in]  ikey        key and value structure
   * @param[in]  dmi         Pointer to DalDmlIntf Class.
   *
   * @retval     UPLL_RC_SUCCESS            Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC        Failure case.
   */
  virtual upll_rc_t UpdateMo(IpcReqRespHeader *req,
                             ConfigKeyVal *ikey,
                             DalDmlIntf *dmi);

  /**
   * @brief     Method used to Rename the VTN and Vnodes during IMPORT operation
   *            when conflicting with corresponding objects in the UNC.
   *
   * @param[in]  req       contains first 8 fields of input request structure
   * @param[in]  key       key and value structure
   * @param[in]  dmi       Pointer to DalDmlIntf Class.
   * @param[in]  ctrlr_id  Controller Id.
   *
   * @retval     UPLL_RC_SUCCESS             Successfull completion.
   * @retval     UPLL_RC_ERR_INSTANCE_EXISTS Failure case.
   * @retval     UPLL_RC_ERR_GENERIC         Failure case.
   */
  virtual upll_rc_t RenameMo(IpcReqRespHeader *req,
                             ConfigKeyVal *key,
                             DalDmlIntf *dmi,
                             const char *ctrlr_id);

  /* @brief         Read the configuration either from RDBMS and/or
   *                from the controller
   *
   * @param[in]     req    Pointer to IpcResResHeader
   * @param[in/out] ikey   Pointer to the ConfigKeyVal Structure
   * @param[in]     dmi    Pointer to the DalDmlIntf(DB Interface)
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
   * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
   *
   **/
  virtual upll_rc_t ReadMo(IpcReqRespHeader *req,
                           ConfigKeyVal *ikey,
                           DalDmlIntf *dmi);

  /* @brief         READ_SIBLING_BEGIN: Gets the first MO from the
   *                sibling group under the parent
   *                specified in the key from the specified UNC database
   *                READ_SIBLING: Gets the next MO from the sibling group
   *                under the parent
   *                specified in the key from the specified UNC database
   *
   * @param[in]     req    Pointer to IpcResResHeader
   * @param[in/out] key    Pointer to the ConfigKeyVal Structure
   * @param[in]     begin  boolean variable to decide the sibling operation
   * @param[in]     dal    Pointer to the DalDmlIntf(DB Interface)
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
   * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
   *
   **/
  virtual upll_rc_t ReadSiblingMo(IpcReqRespHeader *req,
                                  ConfigKeyVal *key,
                                  bool begin,
                                  DalDmlIntf *dal);

  /* @brief       Gets the count of MOs from the sibling group under the parent
   *              specified in the key from the specified UNC database
   *
   * @param[in]     req    Pointer to IpcResResHeader
   * @param[in/out] ikey   Pointer to the ConfigKeyVal Structure
   * @param[in]     dmi    Pointer to the DalDmlIntf(DB Interface)
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
   * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
   *
   **/
  virtual upll_rc_t ReadSiblingCount(IpcReqRespHeader *req,
                                     ConfigKeyVal* ikey,
                                     DalDmlIntf *dmi);

  /* @brief      Validates Read, ReadSibling, ReadSiblingBegin, ReadSiblingcount
   *              and Control requests from VTNService
   * @param[in]     header    Pointer to IpcResResHeader
   * @param[in/out] ikey      Pointer to the ConfigKeyVal Structure
   * @param[in]     dmi       Pointer to the DalDmlIntf(DB Interface)
   * @param[in]     ctrlr_id  Controller name
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
   * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
   *
   **/
  virtual upll_rc_t ReadInfoFromDB(IpcReqRespHeader *header,
                                        ConfigKeyVal* ikey,
                                        DalDmlIntf *dmi,
                                        controller_domain *ctrlr_dom);

  /* @brief     To control operation on key types
   *
   * @param[in]     header    Pointer to IpcResResHeader
   * @param[in/out] ikey      Pointer to the ConfigKeyVal Structure
   * @param[in]     dmi       Pointer to the DalDmlIntf(DB Interface)
   *
   * @retval  UPLL_RC_SUCCESS                    Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC                Generic failure.
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource disconnected.
   * @retval  UPLL_RC_ERR_DB_ACCESS              DB Read/Write error.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE       Given key does not exist
   *
   **/
  virtual upll_rc_t ControlMo(IpcReqRespHeader *header,
                              ConfigKeyVal *ikey,
                              DalDmlIntf *dmi) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

  virtual upll_rc_t TxVote(unc_key_type_t keytype,
                           DalDmlIntf *dmi,
                           TcConfigMode config_mode,
                           std::string vtn_name,
                           ConfigKeyVal **err_ckv);
  virtual upll_rc_t TxVoteCtrlrStatus(unc_key_type_t keytype,
                                      list<CtrlrVoteStatus*> *ctrlr_vote_status,
                                      DalDmlIntf *dmi,
                                      TcConfigMode config_mode,
                                      std::string vtn_name);

  /**
   * @brief  Update controller with the new created / updated / deleted
   * configuration between the Candidate and the Running configuration
   *
   * @param[in]  keytype                  Specifies the keytype
   * @param[in]  session_id               Ipc client session id
   * @param[in]  config_id                Ipc request header config id
   * @param[in]  phase                    Specifies the operation
   * @param[in]  dmi                      Pointer to DalDmlIntf class.
   * @param[out] affected_ctrlr_set       Returns the list of controller to
   *                                      which the command has been delivered.
   *
   * @retval  UPLL_RC_SUCCESS                    Request successfully processed.
   * @retval  UPLL_RC_ERR_GENERIC                Generic error.
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource is diconnected.
   * @retval  UPLL_RC_ERR_DB_ACCESS              DBMS access failure.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE Instance specified does not exist.
   * @retval  UPLL_RC_ERR_INSTANCE_EXISTS  Instance specified already exist.
   */
  virtual upll_rc_t TxUpdateController(unc_key_type_t keytype,
                                       uint32_t session_id, uint32_t config_id,
                                       uuc::UpdateCtrlrPhase phase,
                                       set<string> *affected_ctrlr_set,
                                       DalDmlIntf *dmi,
                                       ConfigKeyVal **err_ckv,
                                       TxUpdateUtil *tx_util,
                                       TcConfigMode config_mode,
                                       std::string vtn_name);

  // To clear c_flag and u_flag in candidate tables
  virtual upll_rc_t TxClearCreateUpdateFlag(unc_key_type_t keytype,
                                            upll_keytype_datatype_t cfg_type,
                                            DalDmlIntf *dmi,
                                            TcConfigMode config_mode,
                                            std::string vtn_name);

  virtual upll_rc_t TxCopyCandidateToRunning(
      unc_key_type_t keytype,
      list<CtrlrCommitStatus*> *ctrlr_commit_status,
      DalDmlIntf *dmi, TcConfigMode config_mode,
      std::string vtn_name);

  virtual upll_rc_t TxEnd(unc_key_type_t keytype, DalDmlIntf *dmi,
                          TcConfigMode config_mode, std::string vtn_name);

  virtual upll_rc_t MergeValidate(unc_key_type_t keytype,
                                  const char *ctrlr_id,
                                  ConfigKeyVal *conflict_ckv,
                                  DalDmlIntf *dmi,
                                  upll_import_type import_type) {
    return UPLL_RC_ERR_NO_SUCH_OPERATION;
  }

  /**
   * @brief     Method used to Copy the datas from IMPORT to CANDIDATE Datatbase
   *
   * @param[in]  key_type                    unc key type.
   * @param[in]  ctrlr_id                    Controller Id.
   * @param[in]  dmi                         Pointer to DalDmlIntf Class.
   *
   * @retval     UPLL_RC_SUCCESS             Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC         Failure case.
   */
  virtual upll_rc_t MergeImportToCandidate(
      unc_key_type_t keytype,
      const char *ctrlr_id,
      DalDmlIntf *dmi,
      upll_import_type = UPLL_IMPORT_TYPE_FULL);

  /**
   * @brief     Method used to Clear the tables in the IMPORT Datatbase.
   *
   * @param[in]  key_type                    unc key type.
   * @param[in]  ctrlr_id                    Controller Id.
   * @param[in]  dmi                         Pointer to DalDmlIntf Class.
   *
   * @retval     UPLL_RC_SUCCESS             Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC         Failure case.
   */
  virtual upll_rc_t ImportClear(unc_key_type_t keytype,
                                const char *ctrlr_id,
                                DalDmlIntf *dmi);

  /**
   * @brief  update controller candidate configuration with the difference in
   *      committed configuration between the UNC and the audited controller
   *
   * @param[in]  keytype     Specifies the keytype.
   * @param[in]  ctrlr_id    Specifies the controller Name.
   * @param[in]  session_id  Ipc client session id.
   * @param[in]  config_id   Ipc request header config id.
   * @param[in]  phase       Specifies the Controller name.
   * @param[in]  dmi         Pointer to DalDmlIntf class.
   *
   * @retval  UPLL_RC_SUCCESS                Request successfully processed.
   * @retval  UPLL_RC_ERR_GENERIC            Generic error.
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource is diconnected.
   * @retval  UPLL_RC_ERR_DB_ACCESS          DBMS access failure.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE   Instance specified does not exist.
   * @retval  UPLL_RC_ERR_INSTANCE_EXISTS    Instance specified already exist.
   */
  virtual upll_rc_t AuditUpdateController(
      unc_key_type_t keytype,
      const char *ctrlr_id,
      uint32_t session_id,
      uint32_t config_id,
      uuc::UpdateCtrlrPhase phase,
      DalDmlIntf *dmi,
      ConfigKeyVal **err_ckv,
      KTxCtrlrAffectedState *ctrlr_affected);
  /**
   * @brief  updates the config status of errored objects returned by the
   *         controller as invalid.
   *
   * @param[in]  keytype      Specifies the keytype.
   * @param[in]  vote_status  Describes the Audit vote status which includes
   *       controller result code, ctrlr_id, errored key
   *       if vote failed
   * @param[in]  dmi          Pointer to DalDmlIntf class.
   *
   * @retval  UPLL_RC_SUCCESS           Request successfully processed.
   * @retval  UPLL_RC_ERR_GENERIC       Generic error.
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource is diconnected.
   * @retval  UPLL_RC_ERR_DB_ACCESS     DBMS access failure.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE Instance specified does not exist.
   * @retval  UPLL_RC_ERR_INSTANCE_EXISTS  Instance specified already exist.
   */
  virtual upll_rc_t AuditVoteCtrlrStatus(unc_key_type_t keytype,
                                         CtrlrVoteStatus *vote_status,
                                         DalDmlIntf *dmi);

  /**
   * @brief  updates the config status of objects which are successfully
   *         committed as applied and errored objects returned by the
   *         controller as invalid.
   *
   * @param[in]  keytype        specifies the keytype.
   * @param[in]  commit_status  Describes the Audit commit status which includes
   *             controller result code, ctrlr_id, errored key
   *             if commit failed
   * @param[in]  dmi            Pointer to DalDmlIntf class.
   *
   * @retval  UPLL_RC_SUCCESS              Request successfully processed.
   * @retval  UPLL_RC_ERR_GENERIC          Generic error.
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource is diconnected.
   * @retval  UPLL_RC_ERR_DB_ACCESS        DBMS access failure.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE Instance specified does not exist.
   * @retval  UPLL_RC_ERR_INSTANCE_EXISTS  Instance specified already exist.
   */
  virtual upll_rc_t AuditCommitCtrlrStatus(unc_key_type_t keytype,
                                           CtrlrCommitStatus *commit_status,
                                           DalDmlIntf *dmi);
  /**
   * @brief  cleans up the MoMgrTbl in the Audit configuration.
   *
   * @param[in]  keytype   specifies the keytype.
   * @param[in]  ctrlr_id  specifies the controller name.
   * @param[in]  dmi       Pointer to DalDmlIntf class.
   *
   * @retval  UPLL_RC_SUCCESS              Request successfully processed.
   * @retval  UPLL_RC_ERR_GENERIC          Generic error.
   * @retval  UPLL_RC_ERR_RESOURCE_DISCONNECTED  Resource is diconnected.
   * @retval  UPLL_RC_ERR_DB_ACCESS        DBMS access failure.
   * @retval  UPLL_RC_ERR_NO_SUCH_INSTANCE Instance specified does not exist.
   * @retval  UPLL_RC_ERR_INSTANCE_EXISTS  Instance specified already exist.
   */
  virtual upll_rc_t AuditEnd(unc_key_type_t keytype,
                             const char *ctrlr_id,
                             DalDmlIntf *dmi);
  virtual upll_rc_t CopyRunningToStartup(unc_key_type_t kt,
                                         DalDmlIntf *dmi);
  virtual upll_rc_t ClearConfiguration(unc_key_type_t kt,
                                  DalDmlIntf *dmi,
                                  upll_keytype_datatype_t cfg_type,
                                  TcConfigMode config_mode,
                                  std::string vtn_name);
  virtual upll_rc_t ClearStartup(unc_key_type_t kt,
                                 DalDmlIntf *dmi);
  /* updates both candidate and running from startup */
  virtual upll_rc_t LoadStartup(unc_key_type_t kt,
                                DalDmlIntf *dmi);
  // Copy entire records from source config to destination config
  virtual upll_rc_t CopyEntireConfiguration(unc_key_type_t kt,
                                            DalDmlIntf *dmi,
                                            upll_keytype_datatype_t dest_cfg,
                                            upll_keytype_datatype_t src_cfg);
  // Initialize the cs to NOT_APPLIED. Used for RUNNING Configuration
  virtual upll_rc_t InitConfigStatus(unc_key_type_t kt,
                                     DalDmlIntf *dmi,
                                     upll_keytype_datatype_t cfg_type);
  virtual upll_rc_t CopyRunningToCandidate(unc_key_type_t kt,
                                           DalDmlIntf *dmi,
                                           unc_keytype_operation_t op,
                                           TcConfigMode config_mode,
                                           std::string vtn_name);
  virtual upll_rc_t IsCandidateDirtyInGlobal(
      unc_key_type_t kt, bool *dirty, DalDmlIntf *dmi,
      bool shallow_check);

  virtual upll_rc_t IsCandidateDirtyShallowInPcm(unc_key_type_t kt,
                                          TcConfigMode config_mode,
                                          std::string vtn_name,
                                          bool *dirty,
                                          DalDmlIntf *dmi);
  virtual upll_rc_t ClearVirtualKtDirtyInGlobal(DalDmlIntf *dmi);

  /*
   *  Rename
   */

  /**
   * @brief     Method used in Rename Operation.
   *            This function collects the Unc new name,
   *            Unc old name and Ctrlr name
   *            informations and creats the configkeyval.
   *
   * @param[in]  ikey                        key and value structure.
   * @param[in]  okey                        key and value structure.
   * @param[out]  rename_info                key and value structure.
   * @param[in]  dmi                         Pointer to DalDmlIntf Class.
   * @param[in]  ctrlr_id                    Controller Id.
   * @param[in]  renamed                     Flag for Already renamed or not.
   *
   * @retval     UPLL_RC_SUCCESS             Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC         Failure case.
   */
  virtual upll_rc_t GetRenameInfo(ConfigKeyVal *ikey,
                                  ConfigKeyVal *okey,
                                  ConfigKeyVal *&rename_info,
                                  DalDmlIntf *dmi,
                                  const char *ctrlr_id,
                                  bool &renamed) {
  UPLL_FUNC_TRACE;
  return UPLL_RC_ERR_GENERIC;
  }

  /**
   * @brief     Method to update the new name in the table while doing
   *            the rename for the specified key type. Based on the KeyType
   *            this function will call the keytype specific function
   *
   *           Ex:
   *              While renaming the VBridge Name, we have to updat the Vlink Tables
   *              so, this function will call the vlink specific function to update the
   *              new VBridge name into the vlink table.
   *
   *
   * @param[in]  req                         contains first 8 fields of input request structure*
   * @param[in]  rename_info                 key and value structure.
   * @param[in]  renamed                     Flag for Already renamed or not.
   * @param[in]  dmi                         Pointer to DalDmlIntf Class.
   *
   * @retval     UPLL_RC_SUCCESS             Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC         Failure case.
   */
  virtual upll_rc_t UpdateTables(IpcReqRespHeader *req,
                                  ConfigKeyVal *&rename_info,
                                  bool &renamed,
                                  DalDmlIntf *dmi,
                                  bool  &no_rename);

  /**
   * @brief     Method used in rename opertaion while update the new name into
   *            the tables to Gets the bindinfo detail for the specifed key type.
   *
   * @param[in]  key_type                    unc key type.
   * @param[out] binfo                       Bindinfo details.
   * @param[out] nattr                       Number of Attributes.
   * @param[in]  tbl                         Table Name.
   *
   * @retval     PFC_TRUE                    Successfull completion.
   */
  virtual bool  GetRenameKeyBindInfo(unc_key_type_t key_type,
                                     BindInfo *&binfo,
                                     int &nattr,
                                     MoMgrTables tbl) = 0;

  /**
   * @brief     Method used in rename opertaion while update the new name into the tables.
   *            This method getting the full key informtion from the Database and Update the
   *            new name and set the rename flag accordingly.
   *
   *            Ex
   *              While renaming the VTN name in VBR_IF, we need the full key to update the
   *              VBR_IF table and set the rename flag accordingly.
   *
   *
   * @param[in/out]  rename_info                 key and value structure.
   * @param[in]  dmi                         Pointer to DalDmlIntf Class.
   * @param[in]  data_type                   Database.
   * @param[in]  renamed                     Flag for Already renamed or not.
   *
   * @retval     UPLL_RC_SUCCESS             Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC         Failure case.
   */
  virtual upll_rc_t UpdateRenamedValue(ConfigKeyVal *&rename_info,
                                       DalDmlIntf *dmi,
                                       upll_keytype_datatype_t data_type,
                                       bool &renamed,
                                       bool &no_rename);


  /**
   * @brief     Method used in rename opertaion while update the new name into the tables.
   *            Based on the Key type this function will update the new name into the table.
   *
   * @param[in]  ikey                        key and value structure.
   * @param[in]  data_type                   Database.
   * @param[in]  dmi                         Pointer to DalDmlIntf Class.
   *
   * @retval     UPLL_RC_SUCCESS             Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC         Failure case.
   */
  virtual upll_rc_t UpdateVnodeTables(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t data_type,
                                      DalDmlIntf *dmi);


  /**
   * @brief     Method create configkey for the specified key type.
   *            Copy the old name from the rename_info into okey.
   *
   * @param[in]  okey                        key and value structure.
   * @param[in]  rename_info                 key and value structure.
   *
   * @retval     UPLL_RC_SUCCESS             Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC         Failure case.
   */
  virtual upll_rc_t CopyToConfigKey(ConfigKeyVal *&okey,
                                    ConfigKeyVal *rename_info) = 0;


  /**
   * @brief     Method used to update the New name for the vnodes which presents in
   *            the Vlink Table.
   *
   * @param[in]  rename_info                 key and value structure.
   * @param[in]  dmi                         Pointer to DalDmlIntf Class.
   * @param[in]  data_type                   Datatbase.
   *
   * @retval     UPLL_RC_SUCCESS             Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC         Failure case.
   */
  virtual upll_rc_t UpdateVnodeVal(ConfigKeyVal *rename_info,
                                   DalDmlIntf *dmi,
                                   upll_keytype_datatype_t data_type,
                                   bool &no_rename) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_ERR_GENERIC;
  }

  /**
   * @brief     Method used in Delete opertaion. Its semantic checks
   *            for the specifed key type.
   *
   * @param[in]  req                         Pointer to IpcResResHeader
   * @param[in]  ikey                        key and value structure.
   * @param[in]  dt_type                     key  type.
   * @param[in]  dmi                         Pointer to DalDmlIntf Class.
   *
   * @retval     UPLL_RC_SUCCESS             Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC         Failure case.
   * @retval     UPLL_RC_ERR_CFG_SEMANTIC    Failue dueto Semantic.
   */
  virtual upll_rc_t IsReferenced(IpcReqRespHeader *req,
                                 ConfigKeyVal *ikey,
                                 DalDmlIntf *dmi) = 0;
  /*
   * @brief     Method used to checks the Vnode presents in the
   *            vlink table.
   *
   * @param[in]  ikey                        key and value structure.
   * @param[in]  dt_type                     key  type.
   * @param[in]  dmi                         Pointer to DalDmlIntf Class.
   *
   * @retval     UPLL_RC_SUCCESS             Vnode not exists in the table.
   * @retval     UPLL_RC_ERR_GENERIC         Failure case.
   * @retval     UPLL_RC_ERR_INSTANCE_EXISTS Vnode exists.
   */
  virtual upll_rc_t CheckVnodeInfo(ConfigKeyVal *ikey,
                                   upll_keytype_datatype_t dt_type,
                                   DalDmlIntf *dmi) {
  return UPLL_RC_ERR_GENERIC;
  }

  /*
   * @brief     Method used to checks the Vnode name is Uniq or not
   *            in all vnodes main table.
   *
   * @param[in]  ikey                         key and value structure.
   * @param[in]  dt_type                      key  type.
   * @param[in]  dmi                          Pointer to DalDmlIntf Class.
   * @param[in]  vexternal_kt                 ikey is vexternal disguised in VBR kt
   *
   * @retval     UPLL_RC_SUCCESS              Vnode not exists in the table.
   * @retval     UPLL_RC_ERR_GENERIC          Failure case.
   * @retval     UPLL_RC_ERR_INSTANCE_EXISTS  Vnode exists.
   * @retval     UPLL_RC_ERR_NO_SUCH_INSTANCE Vnode Doesn't exists.
   */
  upll_rc_t VnodeChecks(ConfigKeyVal *ikey,
                        upll_keytype_datatype_t dt_type,
                        DalDmlIntf *dmi,
                        bool vexternal_kt);
  /*
   * @brief     Method used to checks the Vnode name is Uniq or not
   *            in all vnodes main table during partial Import.
   *
   * @param[in]  ikey                         key and value structure.
   * @param[in]  dt_type                      key  type.
   * @param[in]  dmi                          Pointer to DalDmlIntf Class.
   * @param[in]  ctrlr_id                     Specifies the controller name.
   *
   * @retval     UPLL_RC_SUCCESS              Vnode not exists in the table.
   * @retval     UPLL_RC_ERR_GENERIC          Failure case.
   * @retval     UPLL_RC_ERR_INSTANCE_EXISTS  Vnode exists.
   * @retval     UPLL_RC_ERR_NO_SUCH_INSTANCE Vnode Doesn't exists.
   */
  upll_rc_t PartialImport_VnodeChecks(ConfigKeyVal *ikey,
                                 upll_keytype_datatype_t dt_type,
                                 const char *ctrlr_id,
                                 DalDmlIntf *dmi);
  /**
   * @brief  Creates Vnode MoMgr specific key
   *
   * @param[in]      ikey      ConfigKeyVal pointer
   * @param[in/out]  iokey     ConfigKeyVal pointer
   *
   * @retval UPLL_RC_ERR_GENERIC   generic error
   * @retval UPLL_RC_SUCCESS       Successful
   */
  virtual upll_rc_t CreateVnodeConfigKey(ConfigKeyVal *ikey,
                                 ConfigKeyVal *&okey) {
    return UPLL_RC_ERR_GENERIC;
  }



  /**
   * @brief  get the consolidated configstatus  of all the controllers.
   *
   * @param[in]  cs_status list which contains all controllers config status.
   *
   * @retval  UNC_CS_INVALID            Overall Configuration status is invalid
   * @retval  UNC_CS_APPLIED            Overall Configuration status is applied.
   * @retval  UNC_CS_NOT_APPLIED        Overall Configuration status is not applied.
   * @retval  UNC_CS_PARTIALLY_APPLIED  Configuration is not applied on all controllers
   *    pertaining to this configuration
   * @retval  UNC_CS_UNKNOWN State before the exact status of configuration is known.
   */
  unc_keytype_configstatus_t GetConsolidatedCsStatus(
                                list< unc_keytype_configstatus_t > cs_status);

  /**
   * @brief  compute the consolidated configstatus between all the controllers and current controller.
   *
   * @param[in]  db_status config status of all the controllers.
   * @param[in]  cs_status config status of current controller.
   *
   * @retval  UNC_CS_INVALID            Overall Configuration status is invalid
   * @retval  UNC_CS_APPLIED            Overall Configuration status is applied.
   * @retval  UNC_CS_NOT_APPLIED        Overall Configuration status is not applied.
   * @retval  UNC_CS_PARTIALLY_APPLIED  Configuration is not applied on all controllers
   *     pertaining to this configuration
   * @retval  UNC_CS_UNKNOWN State before the exact status of configuration is known.
   */
  unc_keytype_configstatus_t ComputeStatus(unc_keytype_configstatus_t db_status,
                                          unc_keytype_configstatus_t cs_status);
  virtual upll_rc_t SetConsolidatedStatus(ConfigKeyVal *ikey,
                                  DalDmlIntf *dmi) {
    return UPLL_RC_SUCCESS;
  }

  int get_ntable() {
    return ntable;
  }

  /**
   * @Brief This API is to update(Add or delete) the controller
   *
   * @param[in] vtn_name     vtn name pointer
   * @param[in] ctrlr_dom    Controller Domain pointer
   * @param[in] op           UNC Operation Code
   * @param[in] dmi          Database Intereface pointer
   *
   * @retval UPLL_RC_SUCCESS Successful.
   * @retval UPLL_RC_ERR_GENERIC Generic error.
   * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE Record is not available.
   *
   */
  virtual upll_rc_t UpdateControllerTableForVtn(uint8_t *vtn_name,
                                                controller_domain *ctrlr_dom,
                                                unc_keytype_operation_t op,
                                                upll_keytype_datatype_t dt_type,
                                                DalDmlIntf *dmi,
                                                uint8_t flag,
                                                TcConfigMode config_mode) {
    UPLL_FUNC_TRACE
    return UPLL_RC_ERR_GENERIC;
  }

  virtual upll_rc_t SetVlinkPortmapConfiguration(ConfigKeyVal *ikey,
                                               upll_keytype_datatype_t dt_type,
                                               DalDmlIntf *dmi,
                                               InterfacePortMapInfo flag,
                                               unc_keytype_operation_t oper,
                                               TcConfigMode config_mode,
                                               string vtn_name) {
  return UPLL_RC_ERR_GENERIC;
}

  virtual upll_rc_t RestorePOMInCtrlTbl(ConfigKeyVal *ikey,
                                             upll_keytype_datatype_t dt_type,
                                             MoMgrTables tbl,
                                             DalDmlIntf* dmi) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_SUCCESS;
  }
  /**
   * @Brief This API is to Dump the local rename's structure.
   */
  void DumpRenameInfo(ConfigKeyVal *ikey);
   /**
     * @brief  returns the controller name from ConfigKeyVal
     *
     * @param[in] ikey   ConfigKeyVal pointer
     *
     * @retval uint8_t  pointer
     * @retval NULL     if ConfigKeyVal is empty
     */
    virtual upll_rc_t GetControllerDomainId(ConfigKeyVal *ikey,
                                            controller_domain_t *ctrlr_dom) {
     return UPLL_RC_ERR_GENERIC;
    }
  virtual upll_rc_t IsHostAddrAndPrefixLenInUse(ConfigKeyVal *ckv,
                                                DalDmlIntf *dmi,
                                                IpcReqRespHeader *req) {
    return UPLL_RC_SUCCESS;
  };
  virtual upll_rc_t EnableAdminStatus(ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi,
                                      IpcReqRespHeader *req) {
  return UPLL_RC_SUCCESS;
  };
  virtual upll_rc_t IsAdminStatusEnable(ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi) {
  return UPLL_RC_SUCCESS;
  };
  virtual upll_rc_t DeleteChildrenPOM(ConfigKeyVal *ikey,
                                      upll_keytype_datatype_t dt_type,
                                      DalDmlIntf *dmi,
                                      TcConfigMode config_mode,
                                      string vtn_name) {
    return UPLL_RC_SUCCESS;
  }
  virtual upll_rc_t SetValidAudit(ConfigKeyVal *&ikey) {
    return UPLL_RC_SUCCESS;
  }

  /* This is bind function for import read operation */
  upll_rc_t BindImportDB(ConfigKeyVal *&ikey,
                         DalBindInfo *&db_info,
                         upll_keytype_datatype_t dt_type,
                         MoMgrTables tbl);
  /* This is called during import read operation */
  upll_rc_t ReadImportDB(ConfigKeyVal *&ikey,
                         IpcReqRespHeader *header,
                         DalDmlIntf *dmi);
  /* This is swap the value after read operation in Import table */
  upll_rc_t SwapKey(ConfigKeyVal *&ikey,
                    uint8_t rename);
  upll_rc_t Getvalstnum(ConfigKeyVal *&ikey,
                        uui::IpctSt::IpcStructNum &struct_num);
  upll_rc_t Swapvaltokey(ConfigKeyVal *&ikey,
                        uint8_t rename_flag);
  upll_rc_t CheckExistenceInRenameTable(ConfigKeyVal *&req,
                             upll_keytype_datatype_t dt_type,
                             unc_key_type_t instance_key_type,
                             DalDmlIntf *dmi);
  /**
   * @brief      Method to get a configkeyval of the parent keytype
   *
   * @param[in/out]  okey           pointer to parent ConfigKeyVal
   * @param[in]      ikey           pointer to the child configkeyval from
   * which the parent configkey val is obtained.
   *
   * @retval         UPLL_RC_SUCCESS      Successfull completion.
   * @retval         UPLL_RC_ERR_GENERIC  Failure case.
   **/
  virtual upll_rc_t GetParentConfigKey(ConfigKeyVal *&okey,
                                       ConfigKeyVal *ikey) = 0;
  /**
   * @brief  Allocates for the specified val in the given configuration in the
   * specified table.
   *
   * @param[in]  ck_val   Reference pointer to configval structure allocated.
   * @param[in]  dt_type  specifies the configuration candidate/running/state
   * @param[in]  tbl      specifies if the corresponding table is the  main
   *                      table / controller table or rename table.
   *
   * @retval     UPLL_RC_SUCCESS      Successfull completion.
   * @retval     UPLL_RC_ERR_GENERIC  Failure case.
   **/
  virtual upll_rc_t AllocVal(ConfigVal *&ck_val,
                             upll_keytype_datatype_t dt_type,
                             MoMgrTables tbl = MAINTBL) = 0;

  // Handle all boundary vlanmap request
  // Handle all boundary portmap request
  virtual upll_rc_t BoundaryMapReq(IpcReqRespHeader *req,
                                       ConfigKeyVal *ikey,
                                       ConfigKeyVal *db_vlink,
                                       ConfigKeyVal *vlanmap_ckv,
                                       ConfigKeyVal *uppl_bdry,
                                       DalDmlIntf *dmi) {
    return UPLL_RC_SUCCESS;
  }

  virtual upll_rc_t PerformRedirectTranslationForAudit(ConfigKeyVal *ikey,
                                        DalDmlIntf *dmi,
                                        upll_keytype_datatype_t dt_type);

  virtual upll_rc_t CheckVnodeInterfaceForRedirection(ConfigKeyVal *ikey,
                                        ConfigKeyVal *running_ckv,
                                        DalDmlIntf *dmi,
                                        upll_keytype_datatype_t dt_type,
                                        unc_keytype_operation_t op);

  void AssignVtnVnodeDetailsForRedirection(uint8_t * vtn_name,
                                uint8_t * vnode_name,
                                uint8_t * if_name,
                                ConfigKeyVal *ffe_key);
  /* @brief      Returns portmap information if portmap is valid
   *             Else returns NULL for portmap
   *
   * @param[in]   ikey     Pointer to ConfigKeyVal
   * @param[out]  valid_pm portmap is valid
   * @param[out]  pm       pointer to portmap informtation if valid_pm
   *
   * @retval  UPLL_RC_SUCCESS      Completed successfully.
   * @retval  UPLL_RC_ERR_GENERIC  Generic failure.
   *
   **/
  virtual upll_rc_t GetPortMap(ConfigKeyVal *ikey,
                               uint8_t &valid_pm, val_port_map_t *&pm,
                               uint8_t &valid_admin, uint8_t &admin_status) {
    return UPLL_RC_ERR_NOT_ALLOWED_FOR_THIS_KT;
  }

  template<typename T3>
  upll_rc_t IsLogicalPortAndVlanIdInUse(ConfigKeyVal *ckv,
                                        DalDmlIntf *dmi,
                                        IpcReqRespHeader *req,
                                        bool &match);

  virtual upll_rc_t SetPortmapConfiguration(ConfigKeyVal *ikey,
                                            upll_keytype_datatype_t dt_type,
                                            DalDmlIntf *dmi,
                                            InterfacePortMapInfo flag,
                                            TcConfigMode config_mode,
                                            string vtn_name) {
    return UPLL_RC_ERR_GENERIC;
  }

  virtual  upll_rc_t ValidateNWM(ConfigKeyVal *ikey,
                                 upll_keytype_datatype_t dt_type,
                                 DalDmlIntf *dmi);
  virtual upll_rc_t  SetRedirectNodeAndPortForRead
                                     (ConfigKeyVal *ikey ,
                                      controller_domain  ctrlr_dom,
                                      val_flowfilter_entry* val_entry,
                                      DalDmlIntf *dmi);

  virtual upll_rc_t GetRenamedUncRedirectedNode(
                                      val_flowfilter_entry* val_entry,
                                      DalDmlIntf *dmi ,
                                      uint8_t* ctrlr_id);
  virtual upll_rc_t PerformSemanticCheckForNWM(
                                      ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi ,
                                      upll_keytype_datatype_t dt_type);
  virtual upll_rc_t  SetVtnNameInRedirectNodeAndPortForRead(
                                      ConfigKeyVal *ck_main ,
                                      uint8_t* vnode_vtn_name);
  virtual upll_rc_t VerifyRedirectDestination(ConfigKeyVal *ikey,
                                      DalDmlIntf *dmi,
                                      upll_keytype_datatype_t dt_type) {
    return UPLL_RC_ERR_GENERIC;
  }
  virtual upll_rc_t GetControllerDomainSpan(ConfigKeyVal *ikey,
                                upll_keytype_datatype_t dt_type,
                                DalDmlIntf *dmi) {
    return UPLL_RC_ERR_GENERIC;
  }
  virtual upll_rc_t GetOperation(uuc::UpdateCtrlrPhase phase,
                                 unc_keytype_operation_t &op) {
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t GlobalTxUpdateController(unc_key_type_t keytype,
                                     uint32_t session_id,
                                     uint32_t config_id,
                                     uuc::UpdateCtrlrPhase phase,
                                     set<string> *affected_ctrlr_set,
                                     DalDmlIntf *dmi,
                                     ConfigKeyVal **err_ckv,
                                     TxUpdateUtil *tx_util,
                                     TcConfigMode config_mode,
                                     std::string vtn_name);


  virtual upll_rc_t GetDomainsForController(
        ConfigKeyVal *ckv_drvr,
        ConfigKeyVal *&ctrlr_ckv,
        DalDmlIntf *dmi) {
    return UPLL_RC_ERR_GENERIC;
  }

  upll_rc_t GlobalAuditUpdateController(
      unc_key_type_t keytype,
      const char *ctrlr_id,
      uint32_t session_id,
      uint32_t config_id,
      uuc::UpdateCtrlrPhase phase,
      DalDmlIntf *dmi,
      ConfigKeyVal **err_ckv,
      KTxCtrlrAffectedState *ctrlr_affected);

 /**
   * @brief   Remove the controller specific information from
   *          candidate configuration during partial import
   *
   *
   * @param[in]  keytype    keytype
   * @param[in]  ctrlr_id   pointer to the controller name
   * @param[in]  dmi        DalDmlIntf pointer
   *
   * @retval UPLL_RC_SUCCESS             Successful
   * @retval UPLL_RC_ERR_GENERIC         failed to update the VbrIf
   * @retval UPLL_RC_ERR_DB_DISCONNECT   Databse disconnect error
   */
  upll_rc_t DeleteGlobalConfigInCandidate(unc_key_type_t keytype,
                                        DalDmlIntf *dmi);

  /**
   * @brief   Partiam import merge validate function for the
   *          key types
   *
   *
   * @param[in]  keytype        keytype
   * @param[in]  ctrlr_id       pointer to the controller name
   * @param[out] conflict_ckv   Conflict configkeyval
   * @param[in]  dmi            DalDmlIntf pointer
   *
   * @retval UPLL_RC_SUCCESS             Successful
   * @retval UPLL_RC_ERR_GENERIC         failed to update the VbrIf
   * @retval UPLL_RC_ERR_DB_DISCONNECT   Databse disconnect error
   * @retval UPLL_RC_ERR_MERGE_CONFLICT  Merege conflict error.
   */
  virtual upll_rc_t PartialMergeValidate(unc_key_type_t keytype,
                                  const char *ctrlr_id,
                                  ConfigKeyVal *conflict_ckv,
                                  DalDmlIntf *dmi) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_ERR_NO_SUCH_OPERATION;
  }
  /**
   * @brief   This method is used only by vNode and vLink keytypes
   *          This will convert the PFC name into UNC name.
   *
   *
   * @param[in]  ikey           Input configkeyval
   * @param[in]  dt_type        Database type
   * @param[in]  dmi            DalDmlIntf pointer
   * @param[in]  ctrlr_id       Controller id
   *
   * @retval UPLL_RC_SUCCESS               Successful
   * @retval UPLL_RC_ERR_GENERIC           failed to update the VbrIf
   * @retval UPLL_RC_ERR_DB_DISCONNECT     Databse disconnect error
   * @retval UPLL_RC_ERR_NO_SUCH_INSTANCE  Record not found.
   */
  upll_rc_t GetUncKey(ConfigKeyVal *ikey,
                     upll_keytype_datatype_t dt_type,
                     DalDmlIntf *dmi,
                     const char *ctrlr_id);
 /**
  * @brief   Generate the Auto rename for the Global keytypes
  *          during partial impor time.
  *          Generate Auto rename for vNode and vLink during
  *          candidate create and partial import time
  *
  *
  * @param[in]  ikey         ConfigKeyVal pointer
  * @param[in]  dt_type      specifies the database type
  * @param[in]  ctrlr_dom    pointer to the controller domain
  * @param[in]  dmi          DalDmlIntf pointer
  * @param[in]  auto_rename  Renamed or not
  *
  * @retval UPLL_RC_SUCCESS                Successful
  * @retval UPLL_RC_ERR_GENERIC           falied to update the VbrIf
  * @retval UPLL_RC_ERR_DB_DISCONNECT     Databse disconnect error
  */
  upll_rc_t GenerateAutoName(ConfigKeyVal *&ikey,
                             upll_keytype_datatype_t dt_type,
                             controller_domain_t *ctrlr_dom,
                             DalDmlIntf *dmi,
                             bool *auto_rename,
                             TcConfigMode config_mode,
                             string vtn_id);
 /**
  * @brief  Get the specific vtn_name and vnode_name from ConfigKeyVal
  *
  * @param[in]  ikey        ConfigKeyVal pointer
  * @param[out] vtn_name    vnode vtn name
  * @param[out] vnode_name  vnode specific name
  *
  * @retval UPLL_RC_SUCCESS      Successful
  * @retval UPLL_RC_ERR_GENERIC  failed to retrieve the values
  */
  virtual upll_rc_t GetVnodeName(ConfigKeyVal *ikey,
                               uint8_t *&vtn_name,
                               uint8_t *&vnode_name) {
    UPLL_FUNC_TRACE;
    return UPLL_RC_ERR_GENERIC;
  };

 /**
  * @brief  This method is deside audto rename requried for vNode types
  *  for other keytypes just convert PFC name to UNC name in case of renamed
  *  in candidate configuration
  *
  * @param[in]  ikey         ConfigKeyVal pointer
  * @param[in]  dt_type      specifies the database type
  * @param[in]  dmi          DalDmlIntf pointer
  * @param[in]  auto_rename  Renamed or not
  *
  * @retval UPLL_RC_SUCCESS      Successful
  * @retval UPLL_RC_ERR_GENERIC  failed to retrieve the values
  */

  upll_rc_t AutoRename(ConfigKeyVal *ikey,
                      upll_keytype_datatype_t dt_type,
                      DalDmlIntf *dmi,
                      bool *is_rename);

 /**
  * @brief  This method is used to create VTN during
  * partial import. This function used only by vNodes
  *
  * @param[in]  ikey         ConfigKeyVal pointer
  * @param[in]  dt_type      specifies the database type
  * @param[in]  dmi          DalDmlIntf pointer
  * @param[in]  auto_rename  Renamed or not
  *
  * @retval UPLL_RC_SUCCESS      Successful
  * @retval UPLL_RC_ERR_GENERIC  failed to retrieve the values
  */

  upll_rc_t CreateVtn(ConfigKeyVal *ikey,
                     upll_keytype_datatype_t dt_type,
                     DalDmlIntf *dmi,
                     bool *is_rename);

 /**
  * @brief  This method is used to Copy the rename tables
  * for candidate configuration into import configuration
  *
  *
  * @param[in]  ctrlr_id     Pointer to the controller name
  * @param[in]  dmi          DalDmlIntf pointer
  *
  * @retval UPLL_RC_SUCCESS      Successful
  * @retval UPLL_RC_ERR_GENERIC  failed to retrieve the values
  */
  virtual upll_rc_t CopyRenameTables(const char *ctrlr_id,
                                     upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dom);
 /**
  * @brief  This method is used to Copy the rename tables
  * for candidate configuration into import configuration
  *
  *
  * @param[in]  ctrlr_id     Pointer to the controller name
  * @param[in]  dmi          DalDmlIntf pointer
  *
  * @retval UPLL_RC_SUCCESS      Successful
  * @retval UPLL_RC_ERR_GENERIC  failed to retrieve the values
  */
  virtual upll_rc_t CopyVtnUnifiedTable(upll_keytype_datatype_t dt_type,
                                     DalDmlIntf *dom);
 /**
  * @brief  This method is used to Remove the delete
  * configuration information from import rename table
  * during partial import
  *
  * @param[in]  keytype        keytype
  * @param[in]  ctrlr_id     Pointer to the controller name
  * @param[in]  dmi          DalDmlIntf pointer
  *
  * @retval UPLL_RC_SUCCESS      Successful
  * @retval UPLL_RC_ERR_GENERIC  failed to retrieve the values
  */
  virtual upll_rc_t PurgeRenameTable(unc_key_type_t keytype,
                                             const char *ctrlr_id,
                                             DalDmlIntf *dom);

 /**
  * @brief  This method is used by partial merge validate
  * during partial import
  *
  * @param[in]  keytype        keytype
  * @param[in]  ctrlr_id     Pointer to the controller name
  * @param[out] ikey         Conflict keytype
  * @param[in]  op           Operation
  * @param[in]  nop          Number of operation
  * @param[in]  dmi          DalDmlIntf pointer
  *
  * @retval UPLL_RC_SUCCESS      Successful
  * @retval UPLL_RC_ERR_GENERIC  failed to retrieve the values
  */
  virtual upll_rc_t ValidateImportWithRunning(unc_key_type_t keytype,
                               const char *ctrlr_id, ConfigKeyVal *ikey,
                               unc_keytype_operation_t op[],
                               int nop, DalDmlIntf *dmi);
  upll_rc_t ValidateIpAddress(ConfigKeyVal* ikey,
                              upll_keytype_datatype_t dt_type,
                              DalDmlIntf *dmi);
  upll_rc_t MergeValidateIpAddress(ConfigKeyVal* ikey,
                                   upll_keytype_datatype_t dt_type,
                                   DalDmlIntf *dmi,
                                   const char *ctrlr_id,
                                   upll_import_type import_type);

  upll_rc_t PI_MergeValidate_for_Vtn_Flowfilter(unc_key_type_t keytype,
                                                const char *ctrlr_id,
                                                ConfigKeyVal *conflict_ckv,
                                                DalDmlIntf *dmi);

  upll_rc_t PI_MergeValidate_for_Vtn_Flowfilter_Entry(unc_key_type_t keytype,
                                                const char *ctrlr_id,
                                                ConfigKeyVal *conflict_ckv,
                                                DalDmlIntf *dmi);

  upll_rc_t PI_MergeValidate_for_Vtn_Policingmap(
                                               unc_key_type_t keytype,
                                               const char *ctrlr_id,
                                               ConfigKeyVal *conflict_ckv,
                                               DalDmlIntf *dmi);
  /**
   * @Brief This API is used to do the semantic check with respect to
   * configuration mode specific during commit
   *
   * @param[in]  ikey                          Pointer to ConfigKeyVal Class
   * @param[in]  dmi                           Pointer to DalDmlIntf Class
   * @param[in]  session_id                    Ipc client session id
   * @param[in]  config_id                     Ipc request header config id
   * @param[in]  operation                     Describes the Type of Opeartion
   * @param[in]  keytype                       Specifies the keytype
   *
   * @retval    UPLL_RC_SUCCESS                Successful completion
   * @retval    UPLL_RC_ERR_GENERIC            Generic Error code
   * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE   No Record in DB
   * @retval    UPLL_RC_ERR_INSTANCE_EXISTS    Record exists in DB
   */

  upll_rc_t PerformModeSpecificSemanticCheck(ConfigKeyVal *ikey,
                                             DalDmlIntf *dmi,
                                             uint32_t session_id,
                                             uint32_t config_id,
                                             unc_keytype_operation_t operation,
                                             unc_key_type_t keytype,
                                             TcConfigMode config_mode,
                                             string vtn_name);


  /**CtrlrTypeAndDomainCheck
   * Verifies the controller presence in the candidate or running database
   *
   * @param[in] ikey - ConfigKeyVal pointer
   * @param[in] req - Describes RequestResponderHeaderClass
   *
   * @retval  UPLL_RC_SUCCESS      Successfull completion.
   * @retval  UPLL_RC_ERR_GENERIC  Returned Generic Error. 
   */
  virtual upll_rc_t CtrlrTypeAndDomainCheck(ConfigKeyVal *ikey,
                                            IpcReqRespHeader *req) {
    return UPLL_RC_ERR_GENERIC;
  }
  upll_rc_t CopyVTunnelFromRunningToCandidate(
                          unc_keytype_operation_t op,
                          DalDmlIntf* dmi,
                          TcConfigMode config_mode,
                          std::string cfg_vtn_name);
  /**
   * @Brief This API is used to check the policer name availability
   * in policingprofiletbl in candidate or running DB
   *
   * @param[in] ikey                          Pointer to ConfigKeyVal Class.
   * @param[in] dt_type                       Configuration information.
   * @param[in] dmi                           Pointer to DalDmlIntf Class.
   * @param[in] op                            Describes the Type of Opeartion.
   *
   * @retval    UPLL_RC_SUCCESS               Successful completion.
   * @retval    UPLL_RC_ERR_GENERIC           Generic Error code.
   * @retval    UPLL_RC_ERR_NO_SUCH_INSTANCE  No Record in DB.
   * @retval    UPLL_RC_ERR_INSTANCE_EXISTS   Record exists in DB.
   */

  virtual upll_rc_t IsPolicyProfileReferenced(ConfigKeyVal *ikey,
                                              upll_keytype_datatype_t dt_type,
                                              DalDmlIntf* dmi,
                                              unc_keytype_operation_t op) {
    return UPLL_RC_ERR_GENERIC;
  }

  virtual upll_rc_t CopyKeyToVal(ConfigKeyVal *ikey,
                                 ConfigKeyVal *&okey) {
    return UPLL_RC_ERR_GENERIC;
  }
  virtual upll_rc_t ValidateVtnRename(ConfigKeyVal *ikey,
                                      ConfigKeyVal *tkey, DalDmlIntf *dmi) {
    return UPLL_RC_ERR_GENERIC;
  }

  void PrintMap();
  static std::map<std::string, std::string>auto_rename_;
  static std::map<std::string, std::string>audit_auto_rename_;

  virtual upll_rc_t PurgeCandidate(unc_key_type_t keytype,
                                     const char *ctrlr_id,
                                     DalDmlIntf *dom);

  upll_rc_t GetRenamedUncKeyWoRedirection(ConfigKeyVal *ikey,
    upll_keytype_datatype_t dt_type, DalDmlIntf *dmi, uint8_t *ctrlr_id);

  upll_rc_t GetRenamedUncKeyWithRedirection(unc_key_type_t kt_type,
                                            upll_keytype_datatype_t dt_type,
                                            const char *ctrlr_id,
                                            DalDmlIntf *dmi);

  virtual unc_key_type_t GetVlinkVnodeIfKeyType(ConfigKeyVal *ck_vlink,
                                              int pos ) {
  UPLL_FUNC_TRACE;
  return UNC_KT_ROOT;
}
  upll_rc_t ResetPortMapVlinkFlag(ConfigKeyVal *ikey,
                                  upll_keytype_datatype_t dt_type,
                                  DalDmlIntf *dmi);
  template<typename T1>
  void CheckOperStatus(uint8_t *vtn_name, ConfigVal *ck_val,
                        unc_key_type_t kt_type,
                       controller_domain ctrlr_dom);

upll_rc_t GetFLPPCountQuery(ConfigKeyVal *ikey,
                            unc_key_type_t deletedkt,
                            string &query_string);
  upll_rc_t GetConfigModeInfo(IpcReqRespHeader *req,
                              TcConfigMode &config_mode,
                              string &vtn_name);

  virtual upll_rc_t InstanceExistsInScratchTbl(
      ConfigKeyVal *ikey, TcConfigMode config_mode, string vtn_name,
      DalDmlIntf *dmi) {
    return UPLL_RC_SUCCESS;
  }

  virtual upll_rc_t ComputeCtrlrTblRefCountFromScratchTbl(
      ConfigKeyVal *ikey,
      DalDmlIntf *dmi, upll_keytype_datatype_t dt_type,
      TcConfigMode config_mode, string vtn_name) {
    return UPLL_RC_SUCCESS;
  }

  virtual upll_rc_t ClearScratchTbl(
      TcConfigMode config_mode, string vtn_name,
      DalDmlIntf *dmi, bool is_abort = false) {
    return UPLL_RC_ERR_GENERIC;
  }
  static inline bool IsEqual(const key_vnode_t &keyvbr1,
                             const key_vnode_t &keyvbr2) {
    UPLL_FUNC_TRACE;
    if (0 != strcmp((const char *)keyvbr1.vtn_key.vtn_name,
                    (const char *)keyvbr2.vtn_key.vtn_name)) {
      return false;
    }
    if (0 != strcmp((const char *)keyvbr1.vnode_name,
                    (const char *)keyvbr2.vnode_name)) {
      return false;
    }
    return true;
  }

  static inline bool IsEqual(const key_vnode_if_t &keyvbrif1,
                             const key_vnode_if_t &keyvbrif2) {
    UPLL_FUNC_TRACE;
    if (IsEqual(keyvbrif1.vnode_key, keyvbrif2.vnode_key)) {
      if (0 != strcmp((const char *)keyvbrif1.vnode_if_name,
                      (const char *)keyvbrif2.vnode_if_name)) {
        return false;
      }
    } else {
      return false;
    }
    return true;
  }

  // convert functions
  virtual upll_rc_t ConvertVbr(DalDmlIntf *dmi, IpcReqRespHeader *req,
                         ConfigKeyVal *ikey, TcConfigMode config_mode,
                         string vtn_name,  unc_keytype_operation_t op) {
    return UPLL_RC_ERR_GENERIC;
  }

  virtual upll_rc_t ConvertVbrIf(DalDmlIntf *dmi, bool match_ctrlr_dom,
                                ConfigKeyVal *ikey, TcConfigMode config_mode,
                                string vtn_name, unc_keytype_operation_t op) {
    return UPLL_RC_ERR_GENERIC;
  }

  virtual upll_rc_t ConvertVtunnelIf(ConfigKeyVal *ikey,
                          unc_keytype_operation_t op, TcConfigMode config_mode,
                          string vtn_name,  DalDmlIntf *dmi) {
    return UPLL_RC_ERR_GENERIC;
  }

  virtual upll_rc_t DeleteVtunnelIf(ConfigKeyVal *ikey,
                          TcConfigMode config_mode,
                          string vtn_name,  DalDmlIntf *dmi) {
    return UPLL_RC_ERR_GENERIC;
  }


  virtual  upll_rc_t ConvertVtunnel(ConfigKeyVal *ikey, uint8_t *un_vbr_name,
                          unc_keytype_operation_t op, TcConfigMode config_mode,
                          string vtn_name, DalDmlIntf *dmi) {
    return UPLL_RC_ERR_GENERIC;
  }

  virtual upll_rc_t ConvertVlink(ConfigKeyVal *ikey, unc_keytype_operation_t op,
                                 DalDmlIntf *dmi, TcConfigMode config_mode,
                                 string vtn_name) {
    return UPLL_RC_ERR_GENERIC;
  }

  virtual upll_rc_t DeleteConvertVlink(ConfigKeyVal *ikey, bool match_ctrlr_dom,
                                 DalDmlIntf *dmi, TcConfigMode config_mode,
                                 string vtn_name) {
    return UPLL_RC_ERR_GENERIC;
  }


  virtual upll_rc_t
  MergeConvertVbridgeImportToCandidate(const char *ctrlr_name,
                                       DalDmlIntf *dmi,
                                       upll_import_type import_type) {
    return UPLL_RC_SUCCESS;
  }


  virtual upll_rc_t HandleSpineDomainIdChange(
                        DalDmlIntf *dmi, IpcReqRespHeader *req,
                        ConfigKeyVal *new_spinedom_ckv,
                        ConfigKeyVal *old_vtunnel_ckv) {
    return UPLL_RC_SUCCESS;
  }
  static bool import_unified_exists_;

  upll_rc_t TxCopyConvertTblFromCandidateToRunning(
      unc_keytype_operation_t op, CtrlrCommitStatusList *ctrlr_commit_status,
      DalDmlIntf* dmi, TcConfigMode config_mode, std::string vtn_name);

  upll_rc_t UpdateUVbrConfigStatusFromVtnGwPort(ConfigKeyVal *ckey,
      DalDmlIntf* dmi, TcConfigMode config_mode, std::string vtn_name);
  upll_rc_t GetUniqueVtns(ConfigKeyVal *ikey,
                          std::set<std::string> *vtn_list,
                          DalDmlIntf *dmi);
  upll_rc_t BindInfoForPortEvents(DalBindInfo *dal_bind_info,
                        ConfigKeyVal *ck_vnif);

  /**
   * @Brief This const member function is used to get an import mode
   *        from UpllConfigMgr.
   *
   * @param[in]   None
   * @param[out]  None
   *
   * @retval    UncImportMode which can be
   *                            UNC_IMPORT_ERROR_MODE / UNC_IMPORT_IGNORE_MODE
   */
  UncImportMode GetImportErrorBehaviour(void) const {
    return (uuc::UpllConfigMgr::GetUpllConfigMgr()->GetImportErrorBehaviour());
  }

  virtual upll_rc_t ValidateUpdateMo(ConfigKeyVal *ikey, ConfigKeyVal *db_ckv) {
    return UPLL_RC_ERR_GENERIC;
  }
};  // class MoMgrImpl
}  // namespace kt_momgr
}  // namespace upll
}  // namespace unc

namespace std {
  using unc::upll::kt_momgr::key_vnode_t;
  template<>
  struct less<key_vnode_t>  {
    bool operator()(const key_vnode_t &keyvnode1,
                       const key_vnode_t keyvnode2) const {
      int ret = strcmp((const char *)keyvnode1.vtn_key.vtn_name,
                     (const char *)keyvnode2.vtn_key.vtn_name);
      if (ret == 0) {
        return (strcmp((const char *)keyvnode1.vnode_name,
                     (const char*)keyvnode2.vnode_name) < 0);
      } else {
        return (ret < 0);
      }
    }
  };
}  // namespace std

#endif  // UNC_UPLL_MOMGR_IMPL_H
