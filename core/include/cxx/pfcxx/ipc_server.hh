/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _PFCXX_IPC_SERVER_HH
#define _PFCXX_IPC_SERVER_HH

/*
 * C++ utilities for PFC IPC server.
 */

#include <string>
#include <boost/noncopyable.hpp>
#include <pfc/ipc_server.h>
#include <pfcxx/ipc.hh>
#ifdef  PFC_MODULE_BUILD
#include <pfc/modipc_server.h>
#endif  /* PFC_MODULE_BUILD */

namespace pfc {
namespace core {
namespace ipc {

class ServerEventDesc;

#ifdef  _PFC_LIBPFCXX_IPCSRV_BUILD
class ServerCallbackManager;
#endif   /* _PFC_LIBPFCXX_IPCSRV_BUILD */

/*
 * Inline methods for ServerSession.
 */

/*
 * Constructor.
 */
inline
ServerSession::ServerSession(pfc_ipcsrv_t *srv)
    : _srv(srv)
{
}

/*
 * Set the timeout of the server session by pfc_timespec_t.
 *
 * If the IPC service handler does not return within the period specified
 * by `timeout', the IPC service is considered as timed out.
 * Specifying NULL to `timeout' means the server session timeout should
 * be disabled.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is returned.
 *    Otherwise error number which indicates the cause of error is
 *    returned.
 */
inline int
ServerSession::setTimeout(const pfc_timespec_t *timeout)
{
    return pfc_ipcsrv_settimeout(_srv, timeout);
}

/*
 * Get IPC client address, which identifies IPC client process associated
 * with this server session.
 *
 * Calling/Exit State:
 *    Upon successful completion, the IPC client address associated with
 *    this session is set to `claddr', and zero is returned.
 *    Otherwise error number which indicates the cause of error is
 *    returned.
 */
inline int
ServerSession::getClientAddress(pfc_ipccladdr_t &claddr)
{
    return pfc_ipcsrv_getcladdr(_srv, &claddr);
}

/*
 * Add a PDU to the output stream.
 * All PDUs added to the IPC server session are sent to the client
 * in added order.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is returned.
 *    Otherwise error number which indicates the cause of error is
 *    returned.
 */

// Signed 8-bit value.
inline int
ServerSession::addOutput(int8_t data)
{
    return pfc_ipcsrv_output_int8(_srv, data);
}

inline int
ServerSession::addOutputInt8(int8_t data)
{
    return addOutput(data);
}

// Unsigned 8-bit value.
inline int
ServerSession::addOutput(uint8_t data)
{
    return pfc_ipcsrv_output_uint8(_srv, data);
}

inline int
ServerSession::addOutputUint8(uint8_t data)
{
    return addOutput(data);
}

// Signed 16-bit value.
inline int
ServerSession::addOutput(int16_t data)
{
    return pfc_ipcsrv_output_int16(_srv, data);
}

inline int
ServerSession::addOutputInt16(int16_t data)
{
    return addOutput(data);
}

// Unsigned 16-bit value.
inline int
ServerSession::addOutput(uint16_t data)
{
    return pfc_ipcsrv_output_uint16(_srv, data);
}

inline int
ServerSession::addOutputUint16(uint16_t data)
{
    return addOutput(data);
}

// Signed 32-bit value.
inline int
ServerSession::addOutput(int32_t data)
{
    return pfc_ipcsrv_output_int32(_srv, data);
}

inline int
ServerSession::addOutputInt32(int32_t data)
{
    return addOutput(data);
}

// Unsigned 32-bit value.
inline int
ServerSession::addOutput(uint32_t data)
{
    return pfc_ipcsrv_output_uint32(_srv, data);
}

inline int
ServerSession::addOutputUint32(uint32_t data)
{
    return addOutput(data);
}

// Signed 64-bit value.
inline int
ServerSession::addOutput(int64_t data)
{
    return pfc_ipcsrv_output_int64(_srv, data);
}

inline int
ServerSession::addOutputInt64(int64_t data)
{
    return addOutput(data);
}

// Unsigned 64-bit value.
inline int
ServerSession::addOutput(uint64_t data)
{
    return pfc_ipcsrv_output_uint64(_srv, data);
}

inline int
ServerSession::addOutputUint64(uint64_t data)
{
    return addOutput(data);
}

// Single precision floating point.
inline int
ServerSession::addOutput(float data)
{
    return pfc_ipcsrv_output_float(_srv, data);
}

inline int
ServerSession::addOutputFloat(float data)
{
    return addOutput(data);
}

// Double precision floating point.
inline int
ServerSession::addOutput(double data)
{
    return pfc_ipcsrv_output_double(_srv, data);
}

inline int
ServerSession::addOutputDouble(double data)
{
    return addOutput(data);
}

// IPv4 address.
inline int
ServerSession::addOutput(struct in_addr &data)
{
    return pfc_ipcsrv_output_ipv4(_srv, &data);
}

// IPv6 address.
inline int
ServerSession::addOutput(struct in6_addr &data)
{
    return pfc_ipcsrv_output_ipv6(_srv, &data);
}

// String. (pointer)
inline int
ServerSession::addOutput(const char *data)
{
    return pfc_ipcsrv_output_string(_srv, data);
}

// String. (std::string)
inline int
ServerSession::addOutput(const std::string &data)
{
    return pfc_ipcsrv_output_string(_srv, data.c_str());
}

// Binary data.
inline int
ServerSession::addOutput(const uint8_t *data, uint32_t length)
{
    return pfc_ipcsrv_output_binary(_srv, data, length);
}

// IPC structure specified by pfc_ipcstdef_t.
inline int
ServerSession::addOutput(const pfc_ipcstdef_t &def, pfc_cptr_t data)
{
    return pfc_ipcsrv_output_stdef(_srv, &def, data);
}

// NULL.
inline int
ServerSession::addOutput(void)
{
    return pfc_ipcsrv_output_null(_srv);
}

/*
 * Get argument of the IPC service sent by the client.
 * Argument is represented by array of PDU. Array index, starts from zero,
 * must be specified to `index'.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is returned.
 *    EINVAL is returned if invalid argument is specified.
 *    EPERM is returned if the PDU type at the given index does not match
 *    the method.
 */

// Signed 8-bit integer.
inline int
ServerSession::getArgument(uint32_t index, int8_t &data)
{
    return pfc_ipcsrv_getarg_int8(_srv, index, &data);
}

// Unsigned 8-bit integer.
inline int
ServerSession::getArgument(uint32_t index, uint8_t &data)
{
    return pfc_ipcsrv_getarg_uint8(_srv, index, &data);
}

// Signed 16-bit integer.
inline int
ServerSession::getArgument(uint32_t index, int16_t &data)
{
    return pfc_ipcsrv_getarg_int16(_srv, index, &data);
}

// Unsigned 16-bit integer.
inline int
ServerSession::getArgument(uint32_t index, uint16_t &data)
{
    return pfc_ipcsrv_getarg_uint16(_srv, index, &data);
}

// Signed 32-bit integer.
inline int
ServerSession::getArgument(uint32_t index, int32_t &data)
{
    return pfc_ipcsrv_getarg_int32(_srv, index, &data);
}

// Unsigned 32-bit integer.
inline int
ServerSession::getArgument(uint32_t index, uint32_t &data)
{
    return pfc_ipcsrv_getarg_uint32(_srv, index, &data);
}

// Signed 64-bit integer.
inline int
ServerSession::getArgument(uint32_t index, int64_t &data)
{
    return pfc_ipcsrv_getarg_int64(_srv, index, &data);
}

// Unsigned 64-bit integer.
inline int
ServerSession::getArgument(uint32_t index, uint64_t &data)
{
    return pfc_ipcsrv_getarg_uint64(_srv, index, &data);
}

// Single precision floating point.
inline int
ServerSession::getArgument(uint32_t index, float &data)
{
    return pfc_ipcsrv_getarg_float(_srv, index, &data);
}

// Double precision floating point.
inline int
ServerSession::getArgument(uint32_t index, double &data)
{
    return pfc_ipcsrv_getarg_double(_srv, index, &data);
}

// IPv4 address.
inline int
ServerSession::getArgument(uint32_t index, struct in_addr &data)
{
    return pfc_ipcsrv_getarg_ipv4(_srv, index, &data);
}

// IPv6 address.
inline int
ServerSession::getArgument(uint32_t index, struct in6_addr &data)
{
    return pfc_ipcsrv_getarg_ipv6(_srv, index, &data);
}

// Pointer to string.
//
// Remarks:
//     - Buffer set to `data' is read-only, and it will be freed when the
//       server session is completed.
//
//     - String PDU accepts NULL. So NULL may be set to `data' if the
//       client sends NULL as string PDU.
inline int
ServerSession::getArgument(uint32_t index, const char *&data)
{
    return pfc_ipcsrv_getarg_string(_srv, index, &data);
}

// Binary data.
// A pointer to binary data is set to `data', and its length in bytes is
// set to `length'.
//
// Remarks:
//     - Buffer set to `data' is read-only, and it will be freed when the
//       server session is completed.
//
//     - Binary PDU accepts NULL. So NULL may be set to `data' if the
//       client sends NULL as binary PDU.
inline int
ServerSession::getArgument(uint32_t index, const uint8_t *&data,
                           uint32_t &length)
{
    return pfc_ipcsrv_getarg_binary(_srv, index, &data, &length);
}

// IPC structure specified by pfc_ipcstdef_t.
inline int
ServerSession::getArgument(uint32_t index, const pfc_ipcstdef_t &def,
                           pfc_ptr_t datap)
{
    return pfc_ipcsrv_getarg_stdef(_srv, index, &def, datap);
}

/*
 * Return the number of PDUs sent by the client.
 */
inline uint32_t
ServerSession::getArgCount(void)
{
    return pfc_ipcsrv_getargcount(_srv);
}

/*
 * Determine the PDU type of argument specified by the PDU array index.
 *
 * Calling/Exit State:
 *    Upon successful completion, PDU data type is stored to `type', and
 *    zero is returned.
 *    Otherwise error number which indicates the cause of error is
 *    returned.
 */
inline int
ServerSession::getArgType(uint32_t index, pfc_ipctype_t &type)
{
    return pfc_ipcsrv_getargtype(_srv, index, &type);
}

/*
 * Determine struct name of received additional data at the given index.
 *
 * Calling/Exit State:
 *    Upon successful completion, struct name is set to `name',
 *    and zero is returned.
 *
 *    EINVAL is returned if invalid argument is specified.
 *    EPERM is returned if the type of the specified PDU is not STRUCT.
 *    EPROTO is returned if the PDU is broken.
 */
inline int
ServerSession::getArgStructName(uint32_t index, const char *&name)
{
    return pfc_ipcsrv_getarg_structname(_srv, index, &name);
}

inline int
ServerSession::getArgStructName(uint32_t index, std::string &name)
{
    const char  *nm;
    int  err(getArgStructName(index, nm));

    if (PFC_EXPECT_TRUE(err == 0)) {
        name = nm;
    }

    return err;
}

/*
 * Unset the IPC server session callback specified by `type'.
 */
inline void
ServerSession::unsetCallback(pfc_ipcsrvcb_type_t type)
{
    pfc_ipcsrv_unsetcallback(_srv, type);
}

/*
 * Clear all registered callbacks.
 */
inline void
ServerSession::clearCallbacks(void)
{
    pfc_ipcsrv_clearcallbacks(_srv);
}
/*
 * Import inlined accessors for user-defined IPC structs.
 */
#define _PFCXX_IPC_IMPORT_STRUCT_SERVER
#include <pfcxx/ipc_server_inline.hh>
#undef  _PFCXX_IPC_IMPORT_STRUCT_SERVER

/*
 * IPC event class to be send to the IPC clients.
 *
 * Remarks:
 *    This class is not thread safe.
 *    One ServerEvent instance must be handled by single thread.
 */
class ServerEvent
    : public ServerSession
{
public:
#ifdef  PFC_MODULE_BUILD
    /*
     * Create a new IPC event.
     * This constructor is provided only for PFC module.
     *
     * `type' is an IPC event type to be assigned to a new event.
     *
     * Calling/Exit State:
     *    Upon successful completion, zero is set to `err'.
     *    Otherwise error number which indicates the cause of error is set to
     *    `err'. If a non-zero value is set to `err', this instance must be
     *    discarded immediately.
     */
    ServerEvent(pfc_ipcevtype_t type, int &err)
        : ServerSession(NULL)
    {
        err = pfc_module_ipcevent_create(&_srv, type);
    }
#else   /* !PFC_MODULE_BUILD */
    /*
     * Create a new IPC event.
     *
     * `name' must be an IPC service name, and `type' is an IPC event type to
     *  be assigned to a new event.
     *
     * Calling/Exit State:
     *    Upon successful completion, zero is set to `err'.
     *    Otherwise error number which indicates the cause of error is set to
     *    `err'. If a non-zero value is set to `err', this instance must be
     *    discarded immediately.
     */
    ServerEvent(const char *name, pfc_ipcevtype_t type, int &err)
        : ServerSession(NULL)
    {
        err = pfc_ipcsrv_event_create(&_srv, name, type);
    }

    ServerEvent(std::string &name, pfc_ipcevtype_t type, int &err)
        : ServerSession(NULL)
    {
        err = pfc_ipcsrv_event_create(&_srv, name.c_str(), type);
    }
#endif  /* PFC_MODULE_BUILD */

    ~ServerEvent();

    /*
     * Post an IPC event to registered event listeners.
     *
     * Calling/Exit State:
     *    Upon successful completion, zero is returned.
     *    Otherwise error number which indicates the cause of error is returned.
     *
     * Remarks:
     *    Once this method is called, this instance is no longer usable.
     */
    inline int
    post(void)
    {
        int  err(pfc_ipcsrv_event_post(_srv));

        _srv = NULL;

        return err;
    }

    /*
     * Post an IPC event to the specified IPC client.
     *
     * Calling/Exit State:
     *    Upon successful completion, zero is returned.
     *    Otherwise error number which indicates the cause of error is returned.
     *
     * Remarks:
     *    Once this method is called, this instance is no longer usable.
     */
    inline int
    postTo(const pfc_ipccladdr_t &claddr)
    {
        int  err(pfc_ipcsrv_event_postto(_srv, &claddr));

        _srv = NULL;

        return err;
    }
};

/*
 * IPC event delivery descriptor.
 */
class ServerEventDesc
    : boost::noncopyable
{
public:
    /*
     * Create a new IPC event delivery descriptor associated with the given
     * IPC server event session.
     *
     * IPC event delivery descriptor can be used to wait for completion of
     * delivery of the given IPC event.
     *
     * Calling/Exit State:
     *    Upon successful completion, zero is set to `err'.
     *    Otherwise error number which indicates the cause of error is set
     *    to `err'. If non-zero value is set to `err', this instance must be
     *    discarded immediately.
     */
    ServerEventDesc(ServerEvent &event, int &err)
    {
        err = pfc_ipcsrv_evdesc_create(&_desc, event._srv);
    }

    /*
     * Destroy IPC event delivery descriptor.
     */
    ~ServerEventDesc()
    {
        if (_desc != PFC_IPCEVDESC_INVALID) {
            (void)pfc_ipcsrv_evdesc_destroy(_desc);
        }
    }

    /*
     * Wait for completion of event delivery associated with this event.
     *
     * The timeout expires when the time interval specified by `timeout'
     * passes, as measured by the system monotonic clock. Specifying NULL
     * to `timeout' means an infinite timeout.
     *
     * Calling/Exit State:
     *    Upon successful completion, zero is returned.
     *    Otherwise error number which indicates the cause of error is
     *    returned.
     */
    inline int
    wait(const pfc_timespec_t *timeout = NULL)
    {
        int  err(pfc_ipcsrv_evdesc_wait(_desc, timeout));

        if (PFC_EXPECT_TRUE(err != EBUSY)) {
            _desc = PFC_IPCEVDESC_INVALID;
        }

        return err;
    }

    /*
     * Wait for completion of event delivery associated with this event.
     *
     * The timeout expires when the absolute time specified by `abstime'
     * passes, as measured by the system monotonic clock. Specifying NULL
     * to `abstime' means an infinite timeout.
     *
     * Calling/Exit State:
     *    Upon successful completion, zero is returned.
     *    Otherwise error number which indicates the cause of error is
     *    returned.
     */
    inline int
    waitAbs(const pfc_timespec_t *abstime = NULL)
    {
        int  err(pfc_ipcsrv_evdesc_wait_abs(_desc, abstime));

        if (PFC_EXPECT_TRUE(err != EBUSY)) {
            _desc = PFC_IPCEVDESC_INVALID;
        }

        return err;
    }

    /*
     * Destroy the IPC event delivery descriptor associated with this instance.
     *
     * Calling/Exit State:
     *    Upon successful completion, zero is returned.
     *    Otherwise error number which indicates the cause of error is
     *    returned.
     */
    inline int
    destroy(void)
    {
        int err(pfc_ipcsrv_evdesc_destroy(_desc));

        _desc = PFC_IPCEVDESC_INVALID;

        return err;
    }

private:
    pfc_ipcevdesc_t   _desc;           // IPC event delivery descriptor.
};

/*
 * Base class of IPC server callback.
 */
class ServerCallback
    : boost::noncopyable
{
#ifdef  _PFC_LIBPFCXX_IPCSRV_BUILD
    friend class  ::pfc::core::ipc::ServerCallbackManager;
#endif  /* _PFC_LIBPFCXX_IPCSRV_BUILD */

public:
    /*
     * Constructor.
     * Reference to IPC server session must be specified.
     */
    ServerCallback(ServerSession &sess) : _sess(&sess), _refcnt(0) {}

    /*
     * Destructor of IPC server callback.
     * Destructor will be called when this callback is unregistered.
     */
    virtual ~ServerCallback();

    /*
     * Interface of callback function.
     * The type of callback is passed to `type'.
     */
    virtual void  callback(pfc_ipcsrvcb_type_t type) = 0;

    /*
     * Return the reference to the IPC server session associated with this
     * callback.
     */
    inline ServerSession &
    getSession(void)
    {
        return *_sess;
    }

private:
    /* IPC server session associated with this callback. */
    ServerSession  *_sess;

    /* Reference counter. */
    uint32_t  _refcnt;
};

}       // ipc
}       // core
}       // pfc

#endif  /* !_PFCXX_IPC_SERVER_HH */
