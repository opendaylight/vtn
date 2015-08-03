/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#include <pfc/ipc.h>
#include <netinet/in.h>
#include <string>
#include <map>
#include <list>
#include <vector>

#ifndef _PFCXX_IPC_SERVER_HH
#define _PFCXX_IPC_SERVER_HH

namespace pfc {
namespace core {
namespace ipc {

class ClientSession;
class ServerCallback;
class ServerEvent;

/*
 * C++ wrapper for IPC server session instance.
 */
class ServerSession {
  friend class  ::pfc::core::ipc::ClientSession;
  friend class  ::pfc::core::ipc::ServerEvent;

 public:
  /*
   * Constructor.
   */
  ServerSession();

  /*
   * Destructor.
   */
  virtual ~ServerSession() {
  }

  static void set_rest(uint32_t rest);
  /*
   * Instance methods.
   */
  int   setTimeout(const pfc_timespec_t *timeout = NULL);
  int   getClientAddress(pfc_ipccladdr_t &claddr);
  int   addOutput(int8_t data);
  int   addOutputInt8(int8_t data);
  int   addOutput(uint8_t data);
  int   addOutputUint8(uint8_t data);
  int   addOutput(int16_t data);
  int   addOutputInt16(int16_t data);
  int   addOutput(uint16_t data);
  int   addOutputUint16(uint16_t data);
  int   addOutput(int32_t data);
  int   addOutputInt32(int32_t data);
  int   addOutput(uint32_t data);
  int   addOutputUint32(uint32_t data);
  int   addOutput(int64_t data);
  int   addOutputInt64(int64_t data);
  int   addOutput(uint64_t data);
  int   addOutputUint64(uint64_t data);
  int   addOutput(float data);
  int   addOutputFloat(float data);
  int   addOutput(double data);
  int   addOutputDouble(double data);
  int   addOutput(struct in_addr &data);
  int   addOutput(struct in6_addr &data);
  int   addOutput(const char *data);
  int   addOutput(const std::string &data);
  int   addOutput(const uint8_t *data, uint32_t length);
  int   addOutput(const pfc_ipcstdef_t &def, pfc_cptr_t data);
  int   addOutput(void);

  int addOutput(val_ctr_st& data);
  int addOutput(val_ctr_domain_st& data);
  int addOutput(val_logical_port& data);
  int addOutput(val_switch& data);
  int addOutput(val_port& data);
  int addOutput(val_link& data);
  int addOutput(val_boundary_st& data);
  int addOutput(key_ctr& data);
  int addOutput(val_ctr& data);
  int addOutput(val_ctr_commit_ver& data);
  int addOutput(key_ctr_domain& data);
  int addOutput(val_ctr_domain& data);
  int addOutput(key_logical_port data);
  int addOutput(val_logical_port_st& data);
  int addOutput(key_logical_member_port data);
  int addOutput(key_switch& data);
  int addOutput(val_switch_st& data);
  int addOutput(key_port data);
  int addOutput(val_port_st& data);
  int addOutput(key_link data);
  int addOutput(val_link_st data);
  int addOutput(key_boundary data);
  int addOutput(val_boundary& data);
  int addOutput(key_root& data);
  int addOutput(val_path_fault_alarm_t& data);
  int addOutput(val_port_st_neighbor& data);
  int addOutput(val_port_stats&);
  int addOutput(val_vbr_if_t&);
  int addOutput(key_vbr_if_t&);
  int addOutput(val_vbr_t&);
  int addOutput(key_vbr_t&);
  int addOutput(val_vtn_t&);
  int addOutput(key_vtn_t&);
  int addOutput(key_dataflow&);
  int addOutput(key_vtn_dataflow&);
  int addOutput(val_switch_st_detail_t&);
  int addOutput(val_df_data_flow_st_t&);
  int addOutput(key_ctr_dataflow&);
  int addOutput(key_vtnstation_controller&);
  int addOutput(val_vtnstation_controller_st&);
  int addOutput(val_vtnstation_controller_stat&);
  int addOutput(key_vtn_flowfilter&);
  int addOutput(val_flowfilter&);
  int addOutput(key_dataflow_v2&);
  int addOutput(val_dataflow_v2&);
  int addOutput(val_lm_port_st_neighbor_t&);
  int addOutput(val_logical_port_boundary&);
  int   getArgument(uint32_t index, int8_t &data);
  int   getArgument(uint32_t index, uint8_t &data);
  int   getArgument(uint32_t index, int16_t &data);
  int   getArgument(uint32_t index, uint16_t &data);
  int   getArgument(uint32_t index, int32_t &data);
  int   getArgument(uint32_t index, uint32_t &data);
  int   getArgument(uint32_t index, int64_t &data);
  int   getArgument(uint32_t index, uint64_t &data);
  int   getArgument(uint32_t index, float &data);
  int   getArgument(uint32_t index, double &data);
  int   getArgument(uint32_t index, struct in_addr &data);
  int   getArgument(uint32_t index, struct in6_addr &data);
  int   getArgument(uint32_t index, const char *&data);
  int   getArgument(uint32_t index, const uint8_t *&data, uint32_t &length);
  int   getArgument(uint32_t index, const pfc_ipcstdef_t &def,
                    pfc_ptr_t datap);


  int   getArgument(int, pfcdrv_val_vbr_if_t&);
  int   getArgument(int, key_vbr_if_t&);
  int   getArgument(int, val_vbr_if_t&);
  int   getArgument(int, val_vbr_t&);
  int   getArgument(int, key_vbr_t&);
  int   getArgument(int, val_vtn_t&);
  int   getArgument(int, key_vtn_t&);
  int getArgument(int index, key_ctr_t& key);
  int getArgument(int index, val_ctr_t& key);
  int getArgument(int index, key_ctr_domain_t& key);
  int getArgument(int index, key_boundary_t& key);
  int getArgument(int index, val_boundary_t& key);
  int getArgument(int index, val_ctr_domain_t& key);
  int getArgument(int index, key_logical_port_t& key);
  int getArgument(int index, val_logical_port_st_t& key);
  int getArgument(int index, key_logical_member_port_t& key);
  int getArgument(int index, key_switch_t& key);
  int getArgument(int index, val_switch_st_t& key);
  int getArgument(int index, key_port_t& key);
  int getArgument(int index, val_port_st_t& key);
  int getArgument(int index, key_link_t& key);
  int getArgument(int index, val_link_st_t& key);
  int getArgument(int, key_ctr_dataflow_t&);
  int getArgument(int, key_dataflow_t&);
  int getArgument(int, val_ctr_commit_ver&);
  int getArgument(int, key_vtnstation_controller&);
  int getArgument(int, val_vtnstation_controller_st&);
  int getArgument(int, val_vtnstation_controller_stat&);
  int getArgument(int, key_vtn_flowfilter_t&);
  int getArgument(int, val_flowfilter_t&);
  int getArgument(int, key_dataflow_v2_t&);
  int getArgument(int, val_dataflow_v2_t&);
  int getArgument(int, key_vtn_dataflow_t&);
    uint32_t getArgCount(void);
  int   getArgType(uint32_t index, pfc_ipctype_t &type);
  int   getArgStructName(uint32_t index, const char *&name);
  int   getArgStructName(uint32_t index, std::string &name);

  int   setCallback(pfc_ipcsrvcb_type_t type, ServerCallback *cbp);
  void  unsetCallback(pfc_ipcsrvcb_type_t type);
  void  clearCallbacks(void);

  static void stub_setArgCount(uint32_t argCount);
  static void stub_setArgStructName(uint32_t index, std::string &name);
  static void stub_setArgument(int result);
  static void stub_setArgument(uint32_t index, uint32_t value);
  static void clearStubData();
  static void stub_setArgType(uint32_t index, pfc_ipctype_t ipctype);
  static void stub_setAddOutput(int result);
  static void stub_setArgument_(int set_arg);
  static void stub_setAddOutput(uint32_t value);

 private:
  static std::map<uint32_t, pfc_ipctype_t > arg_parameters;
  static std::map<uint32_t, std::string> structNameMap;
  static std::map<uint32_t, uint32_t> arg_map;
  static std::vector<uint32_t> get_argument_list;
  static std::vector<uint32_t> add_output_list;
  static bool addOutPut_;
  static int result_;
  static uint32_t argCount_;
};

class ServerEvent
: public ServerSession {
 public:
  ServerEvent(const char *name, pfc_ipcevtype_t type, int &err)
      : ServerSession() {
    err = serverEventErr_;
  }

  ServerEvent(std::string &name, pfc_ipcevtype_t type, int &err)
      : ServerSession() {
    err = serverEventErr_;
  }

  ServerEvent(uint8_t type, int &err)
      : ServerSession() {
    err = serverEventErr_;
  }

  ~ServerEvent() {
  }

  inline int
      post(void) {
        return postResult_;
      }

  inline int
      postTo(pfc_ipccladdr_t &claddr) {
        return 0;
      }

  static void clearStubData();
  static void stub_setserverEventErr(int err);
  static void stub_setPostResult(int result);
 private:
  static int serverEventErr_;
  static int postResult_;
};

class ServerCallback {
 public:
  explicit ServerCallback(ServerSession &sess) : _sess(&sess), _refcnt(0) {}
  virtual ~ServerCallback();
  virtual void  callback(pfc_ipcsrvcb_type_t type) = 0;
  inline ServerSession &
      getSession(void) {
        return *_sess;
      }

 private:
  ServerSession  *_sess;
  uint32_t  _refcnt;
};
}  //  namespace ipc
}  //  namespace core
}  //  namespace pfc

#endif  /* !_PFCXX_IPC_SERVER_HH */
