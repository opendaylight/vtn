/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * C++ utilities for PFC IPC server.
 */

#include <string>
#include <pfc/ipc.h>
#include <pfcxx/ipc_server.hh>
#include <iostream>

namespace pfc {
namespace core {
namespace ipc {

std::map<uint32_t,pfc_ipctype_t > ServerSession::arg_parameters;
std::map<uint32_t,std::string> ServerSession::structNameMap;
std::map<uint32_t,uint32_t> ServerSession::arg_map;
std::vector<uint32_t> ServerSession::add_output_list;
bool ServerSession::addOutPut_;
int ServerSession::result_;
uint32_t ServerSession::argCount_;


ServerSession::ServerSession()
{

}

int
ServerSession::setTimeout(const pfc_timespec_t* /*timeout*/)
{
  return 0;
}

int
ServerSession::getClientAddress(pfc_ipccladdr_t& /*&claddr*/)
{
  return 0;
}

// Signed 8-bit value.
int
ServerSession::addOutput(int8_t /*data*/)
{
  return addOutPut_;
}

int
ServerSession::addOutput( )
{
  return addOutPut_;
}

int
ServerSession::addOutputInt8(int8_t /*data*/)
{
  return addOutPut_;
}

// Unsigned 8-bit value.
int
ServerSession::addOutput(uint8_t /*data*/)
{
  return addOutPut_;
}

int
ServerSession::addOutputUint8(uint8_t /*data*/)
{
  return addOutPut_;
}

// Signed 16-bit value.
int
ServerSession::addOutput(int16_t /*data*/)
{
  return addOutPut_;
}

int
ServerSession::addOutputInt16(int16_t /*data*/)
{
  return addOutPut_;
}

// Unsigned 16-bit value.
int
ServerSession::addOutput(uint16_t /*data*/)
{
  return addOutPut_;
}

int
ServerSession::addOutputUint16(uint16_t /*data*/)
{
  return addOutPut_;
}

// Signed 32-bit value.
int
ServerSession::addOutput(int32_t /*data*/)
{
  return addOutPut_;
}

int
ServerSession::addOutputInt32(int32_t /*data*/)
{
  return addOutPut_;
}

// Unsigned 32-bit value.
int
ServerSession::addOutput(uint32_t data)
{
  for (std::vector<uint32_t>::iterator it=add_output_list.begin(); it != add_output_list.end(); ++it)
  {
    if( data == *it )
    {
      return 0;
    }
  }
  return 1;

}

void
ServerSession::stub_setAddOutput(uint32_t data)
{
  add_output_list.push_back(data);
}

int
ServerSession::addOutputUint32(uint32_t /*data*/)
{
  return addOutPut_;
}

// Signed 64-bit value.
int
ServerSession::addOutput(int64_t /*data*/)
{
  return addOutPut_;
}

int
ServerSession::addOutputInt64(int64_t /*data*/)
{
  return addOutPut_;
}

// Unsigned 64-bit value.
int
ServerSession::addOutput(uint64_t /*data*/)
{
  return addOutPut_;
}

int
ServerSession::addOutputUint64(uint64_t /*data*/)
{
  return addOutPut_;
}

// Single precision floating point.
int
ServerSession::addOutput(float /*data*/)
{
  return addOutPut_;
}

int
ServerSession::addOutputFloat(float /*data*/)
{
  return addOutPut_;
}

// Double precision floating point.
int
ServerSession::addOutput(double /*data*/)
{
  return addOutPut_;
}

int
ServerSession::addOutputDouble(double /*data*/)
{
  return addOutPut_;
}

// IPv4 address.
int
ServerSession::addOutput(struct in_addr& /*data*/)
{
  return addOutPut_;
}

// IPv6 address.
int
ServerSession::addOutput(struct in6_addr& /*data*/)
{
  return addOutPut_;
}

// String. (pointer)
int
ServerSession::addOutput(const char* /*data*/)
{
  return addOutPut_;
}

// String. (std::string)
int
ServerSession::addOutput(const std::string& /*data*/)
{
  return addOutPut_;
}

// Binary data.
int
ServerSession::addOutput(const uint8_t* /*data*/, uint32_t /*length*/)
{
  return addOutPut_;
}

// IPC structure specified by pfc_ipcstdef_t.
int
ServerSession::addOutput(const pfc_ipcstdef_t&  /*def*/, pfc_cptr_t /*data*/)
{
  return addOutPut_;
}


int
ServerSession::addOutput(val_ctr_st&)
{
  return addOutPut_;
}

int
ServerSession::addOutput(val_ctr_domain_st&)
{
  return addOutPut_;
}

int
ServerSession::addOutput(val_logical_port&)
{
  return addOutPut_;
}

int
ServerSession::addOutput(val_switch&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(val_port&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(val_link&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(val_boundary_st&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(key_ctr&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(val_ctr&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(key_ctr_domain&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(val_ctr_domain&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(key_logical_port)
{
  return addOutPut_;
}
int
ServerSession::addOutput(val_logical_port_st&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(key_logical_member_port)
{
  return addOutPut_;
}
int
ServerSession::addOutput(key_switch&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(val_switch_st&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(key_port)
{
  return addOutPut_;
}
int
ServerSession::addOutput(val_port_st&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(key_link)
{
  return addOutPut_;
}
int
ServerSession::addOutput(val_link_st)
{
  return addOutPut_;
}
int
ServerSession::addOutput(key_boundary)
{
  return addOutPut_;
}
int
ServerSession::addOutput(val_boundary&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(key_root&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(val_path_fault_alarm_t&)
{
  return addOutPut_;
}
int
ServerSession::addOutput(val_port_st_neighbor&)
{
  return addOutPut_;
}

int ServerSession:: addOutput(key_vbr_if&) {
  return addOutPut_;
}
int ServerSession::addOutput(pfcdrv_val_vbr_if&) {
  return addOutPut_;

}
int ServerSession::addOutput(key_vbr&) {
  return addOutPut_;

}
int ServerSession::addOutput(val_vbr&) {
  return addOutPut_;

}
int ServerSession::addOutput(key_vtn&) {
  return addOutPut_;

}
int ServerSession::addOutput(val_vtn&) {
  return addOutPut_;

}

void
ServerSession::stub_setAddOutput(int result)
{
  addOutPut_ = result;
}
// Signed 8-bit integer.
int
ServerSession::getArgument(uint32_t /*index*/, int8_t&  /*&data*/)
{
  return result_;
}

// Unsigned 8-bit integer.
int
ServerSession::getArgument(uint32_t /*index*/, uint8_t& /*data*/)
{
  return result_;
}

// Signed 16-bit integer.
int
ServerSession::getArgument(uint32_t /*index*/, int16_t& /*&data*/)
{
  return result_;
}

// Unsigned 16-bit integer.
int
ServerSession::getArgument(uint32_t /*index*/, uint16_t& /*&data*/)
{
  return result_;
}

// Signed 32-bit integer.
int
ServerSession::getArgument(uint32_t /*index*/, int32_t& /*&data*/)
{
  return result_;
}

// Unsigned 32-bit integer.
int
ServerSession::getArgument(uint32_t index, uint32_t &data)
{
  if (0 != arg_map.count(index))
  {
    data = arg_map[index];
    return 0;
  }
  return 1;
}

// Signed 64-bit integer.
int
ServerSession::getArgument(uint32_t /*index*/, int64_t& /*data*/)
{
  return result_;
}

// Unsigned 64-bit integer.
int
ServerSession::getArgument(uint32_t /*index*/, uint64_t& /*data*/)
{
  return result_;
}

// Single precision floating point.
int
ServerSession::getArgument(uint32_t /*index*/, float& /*data*/)
{
  return result_;
}

// Double precision floating point.
int
ServerSession::getArgument(uint32_t /*index*/, double& /*data*/)
{
  return result_;
}

// IPv4 address.
int
ServerSession::getArgument(uint32_t /*index*/, struct in_addr& /*data*/)
{
  return result_;
}

// IPv6 address.
int
ServerSession::getArgument(uint32_t /*index*/, struct in6_addr& /*data*/)
{
  return result_;
}

int
ServerSession::getArgument(uint32_t /*index*/, const char*& /*data*/)
{
  return result_;
}


int
ServerSession::getArgument(uint32_t /*index*/, const uint8_t*&  /*data*/,
                           uint32_t& /*length*/)
{
  return result_;
}

int
ServerSession::getArgument(uint32_t /*index*/, const pfc_ipcstdef_t& /*def*/,
                           pfc_ptr_t /*datap*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, key_ctr_t& /*data*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, val_ctr_t& /*data*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, key_ctr_domain_t& /*data*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, key_boundary_t& /*data*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, val_boundary_t& /*data*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, val_ctr_domain_t& /*data*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, key_logical_port_t& /*data*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, val_logical_port_st_t& /*data*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, key_logical_member_port_t& /*data*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, key_switch_t& /*data*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, val_switch_st_t& /*data*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, key_port_t& /*data*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, val_port_st_t& /*data*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, key_link_t& /*data*/)
{
  return result_;
}
int
ServerSession::getArgument(int /*index*/, val_link_st_t& /*data*/)
{
  return result_;
}



int ServerSession:: getArgument(int, key_vbr_if&) {
  return result_;
}
int ServerSession::getArgument(int, pfcdrv_val_vbr_if&) {
  return result_;

}
int ServerSession::getArgument(int, key_vbr&) {
  return result_;

}
int ServerSession::getArgument(int, val_vbr&) {
  return result_;

}
int ServerSession::getArgument(int, key_vtn&) {
  return result_;

}
int ServerSession::getArgument(int, val_vtn&) {
  return result_;

}

void
ServerSession::stub_setArgument( uint32_t index,uint32_t value )
{
  arg_map.insert(std::make_pair(index,value) );
}

void
ServerSession::stub_setArgument(int result)
{
  result_= result;
}

uint32_t
ServerSession::getArgCount(void)
{
  return argCount_;
}

void
ServerSession::stub_setArgCount(uint32_t argCount)
{
  argCount_ = argCount ;
}

int
ServerSession::getArgType(uint32_t index, pfc_ipctype_t& type)
{
  if (0 != arg_parameters.count(index))
  {
    type =arg_parameters[index];
    return 0;
  }
  return 1;
}

void
ServerSession::stub_setArgType(uint32_t index,pfc_ipctype_t ipctype )
{
  arg_parameters.insert(std::make_pair(index,ipctype) );
}

int
ServerSession::getArgStructName(uint32_t index, const char*& name)
{
  if (0 != structNameMap.count(index))
  {
    name = structNameMap[index].c_str();
    return 0;
  }
  return 1;
}

int
ServerSession::getArgStructName(uint32_t index, std::string &name)
{
  if (0 != structNameMap.count(index))
  {
    name = structNameMap[index];
    return 0;
  }
  return 1;
}

void
ServerSession::stub_setArgStructName(uint32_t index, std::string &name)
{
  structNameMap.insert(std::make_pair(index,name));
}

void
ServerSession::unsetCallback(pfc_ipcsrvcb_type_t type)
{

}

void
ServerSession::clearCallbacks(void)
{

}
void
ServerSession::clearStubData(void)
{
  arg_parameters.clear();
  structNameMap.clear();
  arg_map.clear();
  add_output_list.clear();
}

int ServerEvent::serverEventErr_=0;
int ServerEvent::postResult_=1;

void ServerEvent::stub_setserverEventErr(int err)
{
  serverEventErr_  = err;
}

void ServerEvent::clearStubData()
{
  ServerSession::clearStubData();
}

void ServerEvent::stub_setPostResult(int result)
{
  postResult_=result;
}

}
}
}

