/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _PFCXX_IPC_HH
#define _PFCXX_IPC_HH

/*
 * Common definitions for C++ language interaces of PFC IPC framework.
 */
/*
#if     !defined(_PFCXX_IPC_SERVER_HH) && !defined(_PFCXX_IPC_CLIENT_HH)
#error  Never include this file directly!
#endif  // !defined(_PFCXX_IPC_SERVER_HH) && !defined(_PFCXX_IPC_CLIENT_HH)
*/
#include <pfc/ipc.h>
#include <netinet/in.h>
#include <boost/noncopyable.hpp>
#include <string>

namespace pfc {
namespace core {
namespace ipc {

class ServerEvent;
class ServerCallback;
class ClientSession;

/*
 * C++ wrapper for IPC server session instance.
 */
class ServerSession
    : boost::noncopyable {
    friend class  ::pfc::core::ipc::ClientSession;
    friend class  ::pfc::core::ipc::ServerEvent;

 public:
    /*
     * Constructor.
     */
    explicit ServerSession(pfc_ipcsrv_t *srv);

    /*
     * Destructor.
     */
    virtual ~ServerSession() {}

    /*
     * Instance methods.
     */
    int setTimeout(const pfc_timespec_t *timeout = NULL);
    int getClientAddress(pfc_ipccladdr_t &claddr);

    int addOutput(int8_t data);
    int addOutputInt8(int8_t data);
    int addOutput(uint8_t data);
    int addOutputUint8(uint8_t data);
    int addOutput(int16_t data);
    int addOutputInt16(int16_t data);
    int addOutput(uint16_t data);
    int addOutputUint16(uint16_t data);
    int addOutput(int32_t data);
    int addOutputInt32(int32_t data);
    int addOutput(uint32_t data);
    int addOutputUint32(uint32_t data);
    int addOutput(int64_t data);
    int addOutputInt64(int64_t data);
    int addOutput(uint64_t data);
    int addOutputUint64(uint64_t data);
    int addOutput(float data);
    int addOutputFloat(float data);
    int addOutput(double data);
    int addOutputDouble(double data);
    int addOutput(struct in_addr &data);
    int addOutput(struct in6_addr &data);
    int addOutput(const char *data);
    int addOutput(const std::string &data);
    int addOutput(const uint8_t *data, uint32_t length);
    int addOutput(const pfc_ipcstdef_t &def, pfc_cptr_t data);
    int addOutput(void);

    int getArgument(uint32_t index, int8_t &data);
    int getArgument(uint32_t index, uint8_t &data);
    int getArgument(uint32_t index, int16_t &data);
    int getArgument(uint32_t index, uint16_t &data);
    int getArgument(uint32_t index, int32_t &data);
    int getArgument(uint32_t index, uint32_t &data);
    int getArgument(uint32_t index, int64_t &data);
    int getArgument(uint32_t index, uint64_t &data);
    int getArgument(uint32_t index, float &data);
    int getArgument(uint32_t index, double &data);
    int getArgument(uint32_t index, struct in_addr &data);
    int getArgument(uint32_t index, struct in6_addr &data);
    int getArgument(uint32_t index, const char *&data);
    int getArgument(uint32_t index, const uint8_t *&data, uint32_t &length);
    int getArgument(uint32_t index, const pfc_ipcstdef_t &def,
                      pfc_ptr_t datap);

    uint32_t getArgCount(void);
    int getArgType(uint32_t index, pfc_ipctype_t &type);

    int getArgStructName(uint32_t index, const char *&name);
    int getArgStructName(uint32_t index, std::string &name);

/*
 * Import accessor definitions for user-defined IPC structs.
 */
#define _PFCXX_IPC_IMPORT_STRUCT_SERVER
#include <pfcxx/ipc_server_proto.hh>
#undef  _PFCXX_IPC_IMPORT_STRUCT_SERVER

    int setCallback(pfc_ipcsrvcb_type_t type, ServerCallback *cbp);
    void  unsetCallback(pfc_ipcsrvcb_type_t type);
    void  clearCallbacks(void);

// private:
    /* Server session instance. */
    pfc_ipcsrv_t      *_srv;
};

/*
 * C++ wrapper for IPC client session instance.
 */
class ClientSession
    : boost::noncopyable {
    friend class  ::pfc::core::ipc::ServerSession;

 public:
    /*
     * Constructors.
     */
    ClientSession(const char *name, pfc_ipcid_t service, int &err);
    ClientSession(const char *name, pfc_ipcid_t service, int &err,
                  uint32_t flags);
    ClientSession(const std::string &name, pfc_ipcid_t service, int &err);
    ClientSession(const std::string &name, pfc_ipcid_t service, int &err,
                  uint32_t flags);
    ClientSession(pfc_ipcconn_t conn, const char *name, pfc_ipcid_t service,
                  int &err);
    ClientSession(pfc_ipcconn_t conn, const char *name, pfc_ipcid_t service,
                  int &err, uint32_t flags);
    ClientSession(pfc_ipcconn_t conn, const std::string &name,
                  pfc_ipcid_t service, int &err);
    ClientSession(pfc_ipcconn_t conn, const std::string &name,
                  pfc_ipcid_t service, int &err, uint32_t flags);
    explicit ClientSession(pfc_ipcsess_t *sess);

    /*
     * Destructor.
     */
    ~ClientSession();

    /*
     * Instance methods.
     */
    int reset(const char *name, pfc_ipcid_t service);
    int reset(const std::string &name, pfc_ipcid_t service);
    int setTimeout(const pfc_timespec_t *timeout = NULL);
    int invoke(pfc_ipcresp_t &response);
    int cancel(pfc_bool_t discard = PFC_FALSE);
    int forward(ClientSession &sess, uint32_t begin = 0,
                  uint32_t end = UINT32_MAX);
    int forward(ServerSession &sess, uint32_t begin = 0,
                  uint32_t end = UINT32_MAX);
    int forwardTo(ServerSession &sess, uint32_t begin = 0,
                    uint32_t end = UINT32_MAX);

    int addOutput(int8_t data);
    int addOutputInt8(int8_t data);
    int addOutput(uint8_t data);
    int addOutputUint8(uint8_t data);
    int addOutput(int16_t data);
    int addOutputInt16(int16_t data);
    int addOutput(uint16_t data);
    int addOutputUint16(uint16_t data);
    int addOutput(int32_t data);
    int addOutputInt32(int32_t data);
    int addOutput(uint32_t data);
    int addOutputUint32(uint32_t data);
    int addOutput(int64_t data);
    int addOutputInt64(int64_t data);
    int addOutput(uint64_t data);
    int addOutputUint64(uint64_t data);
    int addOutput(float data);
    int addOutputFloat(float data);
    int addOutput(double data);
    int addOutputDouble(double data);
    int addOutput(struct in_addr &data);
    int addOutput(struct in6_addr &data);
    int addOutput(const char *data);
    int addOutput(const std::string &data);
    int addOutput(const uint8_t *data, uint32_t length);
    int addOutput(const pfc_ipcstdef_t &def, pfc_cptr_t data);
    int addOutput(void);

    int getResponse(uint32_t index, int8_t &data);
    int getResponse(uint32_t index, uint8_t &data);
    int getResponse(uint32_t index, int16_t &data);
    int getResponse(uint32_t index, uint16_t &data);
    int getResponse(uint32_t index, int32_t &data);
    int getResponse(uint32_t index, uint32_t &data);
    int getResponse(uint32_t index, int64_t &data);
    int getResponse(uint32_t index, uint64_t &data);
    int getResponse(uint32_t index, float &data);
    int getResponse(uint32_t index, double &data);
    int getResponse(uint32_t index, struct in_addr &data);
    int getResponse(uint32_t index, struct in6_addr &data);
    int getResponse(uint32_t index, const char *&data);
    int getResponse(uint32_t index, const uint8_t *&data, uint32_t &length);
    int getResponse(uint32_t index, const pfc_ipcstdef_t &def,
                      pfc_ptr_t datap);

    uint32_t getResponseCount(void);
    int getResponseType(uint32_t index, pfc_ipctype_t &type);

    int getResponseStructName(uint32_t index, const char *&name);
    int getResponseStructName(uint32_t index, std::string &name);

/*
 * Import accessor definitions for user-defined IPC structs.
 */
#define _PFCXX_IPC_IMPORT_STRUCT_CLIENT
#include <pfcxx/ipc_client_proto.hh>
#undef  _PFCXX_IPC_IMPORT_STRUCT_CLIENT

 private:
    /* Client session instance. NULL is kept on creation error. */
    pfc_ipcsess_t    *_sess;
};

}  // namespace ipc
}  // namespace core
}  // namespace pfc

#endif  /* !_PFCXX_IPC_HH */
