/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef UPLL_IPC_UTIL_HH_
#define UPLL_IPC_UTIL_HH_

#include <string>
#include <map>
#include <list>

#include "pfcxx/synch.hh"

#include "pfcxx/ipc_server.hh"
#include "pfcxx/ipc_client.hh"

#include "unc/keytype.h"
#include "unc/upll_errno.h"

#include "no_copy_assign.hh"

#include "./keytype_upll_ext.h"
#include "ipct_st.hh"
#include "uncxx/upll_log.hh"

#include "upll_validation.hh"

namespace unc {
namespace upll {

static const uint32_t kIpcTimeoutReadState = 300;
static const uint32_t kIpcTimeoutPing = 330;
static const uint32_t kIpcTimeoutImport = 300;
static const uint32_t kIpcTimeoutDataflow = 3600;
static const uint32_t kIpcTimeoutVtnstation = 300;
static const uint32_t kIpcTimeoutCandidate = 600;

namespace ipc_util {

// using pfc::core::ipc::ServerSession;
// using pfc::core::ipc::ClientSession;

static const uint32_t kKeyTreeReqMandatoryFields = 9;
static const uint32_t kKeyTreeRespMandatoryFields = 10;
static const uint32_t kKeyTreeDriverRespMandatoryFields = 12;
static const uint32_t kPhyConfigNotificationMandatoryFields = 4;
static const uint32_t kPfcDrvierAlarmMandatoryFields = 7;

typedef struct key_user_data {
  uint8_t ctrlr_id[unc::upll::kt_momgr::kMaxLenCtrlrId+1];
  uint8_t domain_id[unc::upll::kt_momgr::kMaxLenDomainId+1];
  uint8_t flags;
} key_user_data_t;

// NOTE: ConfigVal assumes the pointers passed in to this class are allocated
// with Malloc() function and calls free() for freeing the memory.
// ConfigVal does not make duplicate memory for the pointers passed.
class ConfigVal {
 public:
  ConfigVal(IpctSt::IpcStructNum st_num = IpctSt::kIpcInvalidStNum,
            void *val = NULL) {
    st_num_ = st_num;
    val_ = val;
    next_cfg_val_ = NULL;
    user_data_ = NULL;
  }
  inline virtual ~ConfigVal() {
    if (val_)
      free(val_);
    val_ = NULL;
    if (user_data_)
      free(user_data_);
    user_data_ = NULL;
    DeleteNextCfgVal();
  }
  inline IpctSt::IpcStructNum get_st_num() const { return st_num_; }
  inline void *get_val() const { return val_; }
  inline void *GetValAndUnlink() {
    void *t = val_;
    val_ = NULL;
    return t;
  }
  inline void SetVal(IpctSt::IpcStructNum st_num, void *val) {
    st_num_ = st_num;
    if (val_) free(val_);
    val_ = val;
  }
  inline ConfigVal *get_next_cfg_val() const { return next_cfg_val_; }
  inline void AppendCfgVal(IpctSt::IpcStructNum st_num, void *val) {
    AppendCfgVal(new ConfigVal(st_num, val));
  }
  void AppendCfgVal(ConfigVal *cfg_val);
  inline void DeleteNextCfgVal() {
    ConfigVal *cv = next_cfg_val_;
    while (cv != NULL) {
      ConfigVal *tmp_cv = cv->next_cfg_val_;
      cv->next_cfg_val_ = NULL;
      delete cv;
      cv = tmp_cv;
    }
    next_cfg_val_ = NULL;
  }

  // user_data is not freed on deleting this object or resetting its value
  inline void *get_user_data() const { return user_data_; }
  inline void set_user_data(void *user_data) { user_data_ = user_data; }
  inline void set_next_cfg_val(ConfigVal *cfg_val) { next_cfg_val_ = cfg_val; }


  // Only dups val structure, nothing else.
  // Note: shallow Dup
  ConfigVal *DupVal() const;

  std::string ToStr() const;
  std::string ToStrAll() const;

 private:
  // std::string ipc_struct_name_;
  IpctSt::IpcStructNum st_num_;
  void *val_;  // { allocated with new operator and points to ipc_struct }
  ConfigVal *next_cfg_val_;  // { allocated with new operator }
  void *user_data_;    // Any data that user wants to store; user manages it
  DISALLOW_COPY_AND_ASSIGN(ConfigVal);
};

// NOTE: ConfigKeyVal assumes the pointers passed in to this class are allocated
// with Malloc() function and calls free() for freeing the memory.
// ConfigKeyVal does not make duplicate memory for the pointers passed.
class ConfigKeyVal {
 public:
  ConfigKeyVal(unc_key_type_t kt,
               IpctSt::IpcStructNum st_num = IpctSt::kIpcInvalidStNum,
               void *key = NULL, ConfigVal *cv = NULL) {
    key_type_ = kt;
    st_num_ = st_num;
    key_ = key;
    cfg_val_ = cv;
    next_ckv_ = NULL;
    user_data_ = NULL;
  }
  virtual ~ConfigKeyVal() {
    if (key_)
      free(key_);
    key_ = NULL;
    DeleteCfgVal();
    DeleteNextCfgKeyVal();
    if (user_data_)
      free(user_data_);
  }

  inline unc_key_type_t get_key_type() const { return key_type_; }
  inline void set_key_type(unc_key_type_t kt) { key_type_ = kt; }
  inline IpctSt::IpcStructNum get_st_num() const { return st_num_; }
  inline void *get_key() const { return key_; }

  inline void SetKey(IpctSt::IpcStructNum st_num, void *key) {
    st_num_ = st_num;
    if (key_) free(key_);
    key_ = key;
  }

  inline void SetUserData(void *user_data) {
    if (user_data_) free(user_data_);
    user_data_ = user_data;
  }
  inline ConfigVal *get_cfg_val() const { return cfg_val_; }

  inline void SetCfgVal(ConfigVal *val) {
    DeleteCfgVal();
    cfg_val_ = val;
  }

  inline void AppendCfgVal(IpctSt::IpcStructNum st_num, void *val) {
    ConfigVal *cv = new ConfigVal(st_num, val);
    AppendCfgVal(cv);
  }

  inline void AppendCfgVal(ConfigVal *cv) {
    if (cfg_val_ == NULL) {
      cfg_val_ = cv;
    } else {
      cfg_val_->AppendCfgVal(cv);
    }
  }
  inline ConfigVal *GetCfgValAndUnlink() {
    ConfigVal *t = cfg_val_;
    cfg_val_ = NULL;
    return t;
  }

  inline void DeleteCfgVal() {
    if (cfg_val_) {
      delete cfg_val_;
      cfg_val_ = NULL;
    }
  }

  inline ConfigKeyVal *get_next_cfg_key_val() const { return next_ckv_; }
  void AppendCfgKeyVal(unc_key_type_t kt, IpctSt::IpcStructNum st_num,
                           void *key, ConfigVal *cv = NULL) {
    AppendCfgKeyVal(new ConfigKeyVal(kt, st_num, key, cv));
  }
  void AppendCfgKeyVal(ConfigKeyVal *cfg_kv);
  // Caller should take care of memory pointed to by next_ckv_, before calling
  // set_next_cfg_key_val
  inline void set_next_cfg_key_val(ConfigKeyVal *ckv) { next_ckv_ = ckv; }
  inline void set_cfg_val(ConfigVal *cfg) { cfg_val_ = cfg; }

  inline void DeleteNextCfgKeyVal() {
    ConfigKeyVal *ckv = next_ckv_;
    while (ckv != NULL) {
      ConfigKeyVal *tmp_ckv = ckv->next_ckv_;
      ckv->next_ckv_ = NULL;
      delete ckv;
      ckv = tmp_ckv;
    }
    next_ckv_ = NULL;
  }

  // user_data is not freed on deleting this object or on resetting its value
  inline void *get_user_data() const { return user_data_; }
  inline void set_user_data(void *user_data) { user_data_ = user_data; }

  ConfigKeyVal *FindFirst(unc_key_type_t keytype) {
    return ((key_type_ ==  keytype) ? this : FindNext(keytype));
  }
  ConfigKeyVal *FindNext(unc_key_type_t keytype);

  size_t size() {
    size_t count = 0;
    for (ConfigKeyVal *ckv = this; ckv; ckv = ckv->next_ckv_) {
      count++;
    }
    return count;
  }

  ConfigKeyVal *LastCfgKeyVal() {
    ConfigKeyVal *ckv = this;
    while (ckv->next_ckv_) {
      ckv = ckv->next_ckv_;
    }
    return ckv;
  }

  // Only dups key and val structures, and not user_data_.
  // Note: shallow Dup
  ConfigKeyVal *DupKey() const;
  // Note: shallow Dup
  ConfigKeyVal *DupKeyVal() const;
  // Note: shallow Dup
  ConfigKeyVal *DupKeyVal(bool dup_key, bool dup_val) const;

  /**
   * Moves data from the argumnent to this instance
   */
  void ResetWith(ConfigKeyVal *from) {
    this->key_type_ = from->key_type_;
    SetKey(from->st_num_, from->key_);
    from->key_ = NULL;
    SetUserData(from->user_data_);
    from->user_data_ = NULL;
    DeleteCfgVal();
    AppendCfgVal(from->cfg_val_);
    from->cfg_val_ = NULL;
    if (this->next_ckv_ != NULL) {
      delete this->next_ckv_;
    }
    this->next_ckv_ = from->next_ckv_;
    from->next_ckv_ = NULL;
  }

  void ResetWithoutNextCkv(ConfigKeyVal *from) {
    this->key_type_ = from->key_type_;
    SetKey(from->st_num_, from->key_);
    from->key_ = NULL;
    SetUserData(from->user_data_);
    from->user_data_ = NULL;
    DeleteCfgVal();
    AppendCfgVal(from->cfg_val_);
    from->cfg_val_ = NULL;
  }

  std::string ToStr() const;
  std::string ToStrAll() const;

  static void *Malloc(size_t size) throw(std::bad_alloc) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
      throw new std::bad_alloc;
    }
    memset(ptr, 0, size);
    return ptr;
  }
  template<class T>
  static T* Malloc() {
    return reinterpret_cast<T *>(ConfigKeyVal::Malloc(sizeof(T)));
  }
  inline static void Free(void *ptr) { free(ptr); }

 private:
  unc_key_type_t key_type_;
  IpctSt::IpcStructNum st_num_;
  void *key_;  // { allocated with Malloc and points to ipc_struct }
  ConfigVal *cfg_val_;  // { allocated with new operator }
  ConfigKeyVal *next_ckv_;  // { allocated with new operator }
  void *user_data_;    // Any data that user wants to store; user manages it
  DISALLOW_COPY_AND_ASSIGN(ConfigKeyVal);
};

class ConfigNotification {
 public:
  // {New and old(optional) values can be given in keyval}
  ConfigNotification(unc_keytype_operation_t operation,
                          unc_keytype_datatype_t datatype,
                          ConfigKeyVal *ckv_data) {
    operation_ = operation;
    datatype_ = datatype;
    ckv_ = ckv_data;
  }

  ~ConfigNotification() {
    if (ckv_)
      delete ckv_;
  }
  inline unc_keytype_operation_t get_operation() const { return operation_; }
  inline unc_keytype_datatype_t get_datatype() const { return datatype_; }
  inline ConfigKeyVal *get_ckv() { return ckv_; }
  inline const ConfigKeyVal *get_ckv() const { return ckv_; }

 private:
  unc_keytype_operation_t operation_;
  unc_keytype_datatype_t datatype_;
  // Val1 in cvk_ is current key, Meaningful in the case of create and update.
  // Val2 in cvk_ is old value. Meaningful in the case of update.
  ConfigKeyVal *ckv_;
};


struct IpcReqRespHeader {
  uint32_t clnt_sess_id;
  uint32_t config_id;
  unc_keytype_operation_t operation;
  uint32_t rep_count;  // {Valid in BULK/SIBLING_BEGIN/SIBLING read opeartions}
  unc_keytype_option1_t option1;
  unc_keytype_option2_t option2;
  upll_keytype_datatype_t datatype;
  upll_rc_t result_code;  // { Valid in response messages only }
};

struct IpcRequest {
  IpcReqRespHeader header;
  ConfigKeyVal *ckv_data;
};

struct IpcResponse {
  IpcReqRespHeader header;
  ConfigKeyVal *ckv_data;
  uint32_t return_code;  // This is the PFC API return code
};

class IpcUtil {
 public:
  static upll_rc_t GetCtrlrTypeFromPhy(const char *ctrlr_name,
                                       upll_keytype_datatype_t dt,
                                       unc_keytype_ctrtype_t *ctrlr_type);
  static upll_rc_t DriverResultCodeToKtURC(unc_keytype_operation_t operation,
                                           uint32_t driver_result_code);
  static bool SendReqToDriver(const char *ctrlr_name, char *domain_id,
                              const char *service_name, pfc_ipcid_t service_id,
                              IpcRequest *req, bool edit_conn,
                              IpcResponse *resp);
  static upll_rc_t PhysicalResultCodeToKtURC(uint32_t result_code);
  static bool SendReqToPhysical(const char *service_name,
                                pfc_ipcid_t service_id,
                                IpcRequest *req, IpcResponse *resp);
  // ctrlr_name and domain_id are valid only when sending to driver. When
  // sending to physical they are ignored.
  static bool SendReqToServer(const char *channel_name,
                              const char *service_name, pfc_ipcid_t service_id,
                              bool driver_msg,
                              const char *ctrlr_name, char *domain_id,
                              IpcRequest *req, IpcResponse *resp);

  static void set_shutting_down(bool shutdown) {
    sys_state_rwlock_.wrlock();
    shutting_down_ = shutdown;
    sys_state_rwlock_.unlock();
  }
  static bool IsShuttingDown() {
    bool state;
    sys_state_rwlock_.rdlock();
    state = shutting_down_;
    sys_state_rwlock_.unlock();
    return state;
  }


  // Allocates memory for *ipc_struct using Malloc. Caller should free memory.
  static bool ReadIpcArg(pfc::core::ipc::ServerSession *sess, uint32_t index,
                         IpctSt::IpcStructNum *st_num, void **ipc_struct);
  // Allocates memory for *ipc_struct using Malloc. Caller should free memory.
  static bool ReadIpcArg(pfc::core::ipc::ClientSession *sess, uint32_t index,
                         IpctSt::IpcStructNum *st_num, void **ipc_struct);
  // Allocates memory for *ipc_struct using Malloc. Caller should free memory.
  static bool ReadIpcStruct(pfc::core::ipc::ServerSession *sess,
                            uint32_t index, IpctSt::IpcStructNum *st_num,
                            void **ipc_struct);
  // Allocates memory for *ipc_struct using Malloc. Caller should free memory.
  static bool ReadIpcStruct(pfc::core::ipc::ClientSession *sess,
                            uint32_t index, IpctSt::IpcStructNum *st_num,
                            void **ipc_struct);
  static bool WriteIpcArg(pfc::core::ipc::ServerSession *sess,
                          IpctSt::IpcStructNum st_num, const void *ipc_struct);
  static bool WriteIpcArg(pfc::core::ipc::ClientSession *sess,
                          IpctSt::IpcStructNum st_num, const void *ipc_struct);
  static bool WriteIpcStruct(pfc::core::ipc::ServerSession *sess,
                             IpctSt::IpcStructNum st_num,
                             const void *ipc_struct);
  static bool WriteIpcStruct(pfc::core::ipc::ClientSession *sess,
                             IpctSt::IpcStructNum st_num,
                             const void *ipc_struct);
  static bool ReadKtRequest(pfc::core::ipc::ServerSession *sess,
                            pfc_ipcid_t service,
                            IpcReqRespHeader *msg_hdr,
                            ConfigKeyVal **first_ckv);
  static bool WriteKtResponse(pfc::core::ipc::ServerSession *sess,
                              const IpcReqRespHeader &msg_hdr,
                              const ConfigKeyVal *first_ckv);
  static bool ReadKtResponse(pfc::core::ipc::ClientSession *sess,
                             pfc_ipcid_t service,
                             bool driver_msg, char *domain_id,
                             IpcReqRespHeader *msg_hdr,
                             ConfigKeyVal **first_ckv);
  static bool WriteKtRequest(pfc::core::ipc::ClientSession *sess,
                             bool driver_msg,
                             const char *ctrlr_name, const char *domain_id,
                             const IpcReqRespHeader &msg_hdr,
                             const ConfigKeyVal *first_ckv);
  static std::string IpcRequestToStr(const IpcReqRespHeader &msghdr);
  static std::string IpcResponseToStr(const IpcReqRespHeader &msghdr);


 private:
  IpcUtil() {}
  ~IpcUtil() {}

  static bool shutting_down_;
  static pfc::core::ReadWriteLock sys_state_rwlock_;

  DISALLOW_COPY_AND_ASSIGN(IpcUtil);
};

class ConfigNotifier {
 public:
  // TX time notification utilities
  // BufferNotificationToUpllUser is used by MoMgr implementations.
  static bool BufferNotificationToUpllUser(ConfigNotification *notif);
  // Send/CancelBufferedNotificationsToUpllUser are used by framework
  static bool SendBufferedNotificationsToUpllUser();
  static bool CancelBufferedNotificationsToUpllUser();
  // Send operstatus notifications immediately.
  static bool SendOperStatusNotification(const ConfigKeyVal *ckv);
 private:
  static std::list<ConfigNotification*> buffered_notifs;
  static pfc::core::Mutex notif_lock;
};

}  // namespace ipc_util
}  // namespace upll
}  // namespace unc

#endif  // UPLL_IPC_UTIL_HH_
