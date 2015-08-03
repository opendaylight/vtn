/*
 * Copyright (c) 2012-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * C++ utilities for PFC IPC client.
 */

#include <pfc/ipc.h>
#include <pfcxx/ipc_client.hh>
#include <pfc/ipc_client.h>
#include <string>

namespace pfc {
namespace core {
namespace ipc {

std::map<uint32_t, pfc_ipctype_t > ClientSession::arg_parameters;
std::map<uint32_t, std::string> ClientSession::structNameMap;
std::map<uint32_t, uint32_t> ClientSession::arg_map;
std::list<uint32_t> ClientSession::add_output_list;
int ClientSession::addOutPut_;
int ClientSession::responseResult_;
int ClientSession::ClientsesErrorCode_;
int ClientSession::argCount_;
pfc_ipcresp_t  ClientSession::ipcresp_;
int ClientSession::err_;
std::list<const char*> ClientSession::add_output_str;

// pfc_hostaddr_t IpcEvent::_local_addr;
// bool  IpcEvent::_addr_initialized = false;


ClientSession::ClientSession(const char* /*name*/,
                                     pfc_ipcid_t /*service*/, int &err) {
  err = 0;
}


ClientSession::ClientSession(const char* /*name*/,
                                         pfc_ipcid_t /*service*/, int &err,
                                         uint32_t /*flags*/) {
  err = 0;
}

ClientSession::ClientSession(const std::string& /*name*/,
                                                    pfc_ipcid_t /*service*/,
                                                    int &err) {
  err = 0;
}

ClientSession::ClientSession(const std::string& /*name*/,
                                               pfc_ipcid_t /*service*/,
                                           int &err, uint32_t /*flags*/) {
  err = 0;
}

ClientSession::ClientSession(pfc_ipcconn_t /*conn*/, const char* /*name*/,
                             pfc_ipcid_t service, int &err) {
  if (service > 0 && service <=3) {
    err = 0;
  } else {
     err = 1;
}
}

ClientSession::ClientSession(pfc_ipcconn_t /*conn*/, const char* /*name*/,
                      pfc_ipcid_t /*service*/,
                      int &err, uint32_t /*flags*/) {
  err = 0;
}

ClientSession::ClientSession(pfc_ipcconn_t /*conn*/,
                             const std::string& /*name*/,
                             pfc_ipcid_t /*service*/, int &err) {
  err = 0;
}


ClientSession::ClientSession(pfc_ipcconn_t /*conn*/,
                   const std::string& /*name*/,
                   pfc_ipcid_t /*service*/, int &err, uint32_t /*flags*/) {
  err = 0;
}

void
ClientSession::stub_setClientSessionErrorCode(int result) {
  ClientsesErrorCode_ = result;
}


ClientSession::ClientSession() {
}

ClientSession::ClientSession(pfc_ipcsess_t *sess) {
}


/*
 * Destructor of the IPC client session instance.
 */

ClientSession::~ClientSession() {
  arg_parameters.clear();
}

int
ClientSession::reset(const char* /*name*/, pfc_ipcid_t /*service*/) {
  return 0;
}

int
ClientSession::reset(const std::string& /*name*/, pfc_ipcid_t /*service*/) {
  return 0;
}

int
ClientSession::setTimeout(const pfc_timespec_t* /*timeout*/) {
  return 0;
}

int
ClientSession::invoke(pfc_ipcresp_t& response) {
  response = ipcresp_;
  return err_;
}

void ClientSession::stub_setinvoke(pfc_ipcresp_t ipcresp, int err) {
  ipcresp_= ipcresp;
  err_= err;
}
int
ClientSession::cancel(pfc_bool_t /*discard*/) {
  return 0;
}

int
ClientSession::forward(ClientSession &sess, uint32_t begin, uint32_t end) {
  return 0;
}

int
ClientSession::forward(ServerSession &sess, uint32_t begin, uint32_t end) {
  return 0;
}

int
ClientSession::forwardTo(ServerSession &sess, uint32_t begin, uint32_t end) {
  return 0;
}

// Signed 8-bit value.
int
ClientSession::addOutput(int8_t /*data*/) {
  return addOutPut_;
}
int
ClientSession::addOutput(void) {
  return addOutPut_;
}
int
ClientSession::addOutputInt8(int8_t /*data*/) {
  return addOutPut_;
}

// Unsigned 8-bit value.
int
ClientSession::addOutput(uint8_t /*data*/) {
  return addOutPut_;
}

int
ClientSession::addOutputUint8(uint8_t /*data*/) {
  return addOutPut_;
}

// Signed 16-bit value.
int
ClientSession::addOutput(int16_t /*data*/) {
  return addOutPut_;
}

int
ClientSession::addOutputInt16(int16_t /*data*/) {
  return addOutPut_;
}

// Unsigned 16-bit value.
int
ClientSession::addOutput(uint16_t /*data*/) {
  return addOutPut_;
}

int
ClientSession::addOutputUint16(uint16_t /*data*/) {
  return addOutPut_;
}

// Signed 32-bit value.
int
ClientSession::addOutput(int32_t /*data*/) {
  return addOutPut_;
}

int
ClientSession::addOutputInt32(int32_t /*data*/) {
  return addOutPut_;
}

// Unsigned 32-bit value.
int
ClientSession::addOutput(uint32_t data) {
  for (std::list<uint32_t>::iterator it = add_output_list.begin();
      it != add_output_list.end(); ++it) {
    uint32_t data_l = *it;
    if (data == data_l) {
      return 0;
    }
  }
  return 1;
}

int
ClientSession::addOutput(key_ctr&) {
  return addOutPut_;
}

int
ClientSession::addOutput(val_ctr&) {
  return addOutPut_;
}

int
ClientSession::addOutput(key_boundary_t&) {
  return addOutPut_;
}

int
ClientSession::addOutput(key_logical_port) {
  return addOutPut_;
}

int
ClientSession::addOutput(val_logical_port&) {
  return addOutPut_;
}

int
ClientSession::addOutput(val_logical_port_st&) {
  return addOutPut_;
}

int
ClientSession::addOutput(key_port&) {
  return addOutPut_;
}

int
ClientSession::addOutput(key_switch&) {
  return addOutPut_;
}

int
ClientSession::addOutput(key_ctr_dataflow&) {
  return addOutPut_;
}

int
ClientSession::addOutput(val_ctr_commit_ver&) {
  return addOutPut_;
}

int
ClientSession::addOutput(key_dataflow&) {
  return addOutPut_;
}

void
ClientSession::stub_setAddOutput(uint32_t data) {
  add_output_list.push_front(data);
}


int
ClientSession::addOutputUint32(uint32_t /*data*/) {
  return addOutPut_;
}

// Signed 64-bit value.
int
ClientSession::addOutput(int64_t /*data*/) {
  return addOutPut_;
}

int
ClientSession::addOutputInt64(int64_t /*data*/) {
  return addOutPut_;
}

// Unsigned 64-bit value.
int
ClientSession::addOutput(uint64_t /*data*/) {
  return addOutPut_;
}

int
ClientSession::addOutputUint64(uint64_t /*data*/) {
  return addOutPut_;
}

// Single precision floating point.
int
ClientSession::addOutput(float /*data*/) {
  return addOutPut_;
}

int
ClientSession::addOutputFloat(float /*data*/) {
  return addOutPut_;
}

// Double precision floating point.
int
ClientSession::addOutput(double /*data*/) {
  return addOutPut_;
}

int
ClientSession::addOutputDouble(double /*data*/) {
  return addOutPut_;
}

// IPv4 address.
int
ClientSession::addOutput(struct in_addr& /*data*/) {
  return addOutPut_;
}

// IPv6 address.
int
ClientSession::addOutput(struct in6_addr& /*data*/) {
  return addOutPut_;
}

// String. (pointer)
int
ClientSession::addOutput(const char* data) {
  for (std::list<const char*>::iterator it = add_output_str.begin();
       it != add_output_str.end(); ++it) {
    const char* data_l = *it;
    std::string str(data);
    if (0 == str.compare(data_l)) {
      return 0;
    }
  }
  return 1;
}


void ClientSession::stub_setAddOutput(const char* data) {
  add_output_str.push_front(data);
}

// String. (std::string)
int
ClientSession::addOutput(const std::string& /*data*/) {
  return addOutPut_;
}

// Binary data.
int
ClientSession::addOutput(const uint8_t* /**data*/, uint32_t /*length*/) {
  return addOutPut_;
}

// IPC structure specified by pfc_ipcstdef_t.
int
ClientSession::addOutput(const pfc_ipcstdef_t& /*def*/, pfc_cptr_t /*data*/) {
  return addOutPut_;
}


void
ClientSession::stub_setAddOutput(int result) {
  addOutPut_ = result;
}

// Signed 8-bit integer.
int
ClientSession::getResponse(uint32_t /*index*/, int8_t& /*data*/) {
  return responseResult_;
}

// Unsigned 8-bit integer.
int
ClientSession::getResponse(uint32_t /*index*/, uint8_t& /*data*/) {
  return responseResult_;
}

// Signed 16-bit integer.
int
ClientSession::getResponse(uint32_t /*index*/, int16_t& /*data*/) {
  return responseResult_;
}

// Unsigned 16-bit integer.
int
ClientSession::getResponse(uint32_t /*index*/, uint16_t& /*data*/) {
  return responseResult_;
}

// Signed 32-bit integer.
int
ClientSession::getResponse(uint32_t /*index*/, int32_t& /*data*/) {
  return responseResult_;
}

// Unsigned 32-bit integer.
int
ClientSession::getResponse(uint32_t index, uint32_t& data) {
  if (0 != arg_map.count(index)) {
    data = arg_map[index];
    return 0;
  }
  return 1;
}

// Signed 64-bit integer.
int
ClientSession::getResponse(uint32_t /*index*/, int64_t& /*data*/) {
  return responseResult_;
}

// Unsigned 64-bit integer.
int
ClientSession::getResponse(uint32_t /*index*/, uint64_t& /*data*/) {
  return responseResult_;
}

// Single precision floating point.
int
ClientSession::getResponse(uint32_t /*index*/, float& /*data*/) {
  return responseResult_;
}

// Double precision floating point.
int
ClientSession::getResponse(uint32_t /*index*/, double& /*data*/) {
  return responseResult_;
}

// IPv4 address.
int
ClientSession::getResponse(uint32_t /*index*/, struct in_addr& /*data*/) {
  return responseResult_;
}

// IPv6 address.
int
ClientSession::getResponse(uint32_t /*index*/, struct in6_addr& /*data*/) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t /*index*/, const char*& /*data*/) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t /*index*/, const uint8_t*& /*data*/,
                           uint32_t& /*length*/) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t /*index*/, const pfc_ipcstdef_t& /*def*/,
                           pfc_ptr_t /*datap*/) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t index, key_ctr& data) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t index, val_phys_path_fault_alarm_t& data) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t index,  key_ctr_domain_t& data) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t index, key_switch_t& data) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t index, key_port_t& data) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t index, val_port_st& data) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t index, val_switch_st& data) {
  return responseResult_;
}
int
ClientSession::getResponse(uint32_t index, key_link_t& data) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t index, val_link_st& data) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t index, val_ctr_st& data) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t index, val_ctr_domain_st& data) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t index, key_logical_port_t& data) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t index, val_logical_port_st& data) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t index, key_logical_member_port_t& data) {
  return responseResult_;
}

int
ClientSession::getResponse(unsigned int, val_port_stats& data) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t, val_switch_st_detail&) {
  return responseResult_;
}

int
ClientSession::getResponse(uint32_t, val_df_data_flow_st_t&) {
  return responseResult_;
}

int
ClientSession::getResponse(unsigned int, key_dataflow&) {
  return responseResult_;
}

int
ClientSession::getResponse(unsigned int, key_vtn&) {
  return responseResult_;
}

int
ClientSession::getResponse(unsigned int, pfcdrv_network_mon_alarm_data&) {
  return responseResult_;
}

int
ClientSession::getResponse(unsigned int, pfcdrv_policier_alarm_data&) {
  return responseResult_;
}
int
ClientSession::getResponse(uint32_t, key_ctr_dataflow_t&) {
  return responseResult_;
}
int
ClientSession::getResponse(unsigned int, val_port_st_neighbor&){
  return responseResult_;
}
void
ClientSession::stub_setResponse(int result) {
  responseResult_ = result;
}

void
ClientSession::stub_setResponse(uint32_t index, uint32_t value) {
  arg_map.insert(std::make_pair(index, value) );
}

uint32_t
ClientSession::getResponseCount(void) {
  return argCount_;
}

void
ClientSession::stub_setResponseCount(int argCount) {
  argCount_= argCount;
}

int
ClientSession::getResponseType(uint32_t index, pfc_ipctype_t& type) {
  if (0 != arg_parameters.count(index)) {
    type = arg_parameters[index];
    return 0;
  }
  return 1;
}

void
ClientSession::stub_setResponsetype(uint32_t index, pfc_ipctype_t ipctype) {
  arg_parameters.insert(std::make_pair(index, ipctype) );
}

int
ClientSession::getResponseStructName(uint32_t index, const char*& name) {
  if (0 != structNameMap.count(index)) {
    name = structNameMap[index].c_str();
    return 0;
  }
  return 1;
}

int
ClientSession::getResponseStructName(uint32_t index, std::string& name) {
  if (0 != structNameMap.count(index)) {
    name = structNameMap[index];
    return 0;
  }
  return 1;
}

void
ClientSession::stub_setResponseStructName(uint32_t index, std::string& name) {
  structNameMap.insert(std::make_pair(index, name));
}

void
ClientSession::clearStubData(void) {
  arg_parameters.clear();
  structNameMap.clear();
  arg_map.clear();
  add_output_list.clear();
  add_output_str.clear();
}
}       //   namespace ipc
}       //  namespace core
}       //  namespace pfc
