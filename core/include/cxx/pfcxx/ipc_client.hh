/*
 * Copyright (c) 2011-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef _PFCXX_IPC_CLIENT_HH
#define _PFCXX_IPC_CLIENT_HH

/*
 * C++ utilities for PFC IPC client.
 */

#include <string>
#include <boost/noncopyable.hpp>
#include <pfc/ipc_client.h>
#include <pfcxx/ipc.hh>

namespace pfc {
namespace core {
namespace ipc {

/*
 * Inline methods for ClientSession.
 */

/*
 * Create a new client session on the default connection, which is always
 * connected to the default IPC channel.
 *
 * `name' must be a non-NULL string which represents IPC service name
 * on the default IPC channel.
 * `service' is a service identifier used to distinguish IPC service.
 *
 * `flags' determines the behavior of the IPC client session.
 * See comments on PFC_IPCSSF_XXX flags in ipc_client.h.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is set to `err'.
 *
 *    Otherwise error number which indicates the cause of error is set
 *    to `err'. If a non-zero value is set to `err', this instance must
 *    be discarded immediately.
 */
inline
ClientSession::ClientSession(const char *name, pfc_ipcid_t service, int &err)
{
    err = pfc_ipcclnt_sess_create(&_sess, name, service);
}

inline
ClientSession::ClientSession(const char *name, pfc_ipcid_t service, int &err,
                             uint32_t flags)
{
    err = pfc_ipcclnt_sess_create4(&_sess, name, service, flags);
}

/*
 * Create a new client session on the default connection, which is always
 * connected to the default IPC channel.
 *
 * This constructor uses std::string to specify IPC service name.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is set to `err'.
 *
 *    Otherwise error number which indicates the cause of error is set
 *    to `err'. If a non-zero value is set to `err', this instance must
 *    be discarded immediately.
 */
inline
ClientSession::ClientSession(const std::string &name, pfc_ipcid_t service,
                             int &err)
{
    err = pfc_ipcclnt_sess_create(&_sess, name.c_str(), service);
}

inline
ClientSession::ClientSession(const std::string &name, pfc_ipcid_t service,
                             int &err, uint32_t flags)
{
    err = pfc_ipcclnt_sess_create4(&_sess, name.c_str(), service, flags);
}

/*
 * Create a new client session on the given connection handle.
 *
 * `conn' must be a connection handle opened by the call of
 * pfc_ipcclnt_sess_altcreate().
 * `name' must be a non-NULL string which represents IPC service name
 * on the IPC channel associated with the given connection handle.
 * `service' is a service identifier used to distinguish IPC service.
 *
 * `flags' determines the behavior of the IPC client session.
 * See comments on PFC_IPCSSF_XXX flags in ipc_client.h.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is set to `err'.
 *
 *    Otherwise error number which indicates the cause of error is set
 *    to `err'. If a non-zero value is set to `err', this instance must
 *    be discarded immediately.
 */
inline
ClientSession::ClientSession(pfc_ipcconn_t conn, const char *name,
                             pfc_ipcid_t service, int &err)
{
    err = pfc_ipcclnt_sess_altcreate(&_sess, conn, name, service);
}

inline
ClientSession::ClientSession(pfc_ipcconn_t conn, const char *name,
                             pfc_ipcid_t service, int &err, uint32_t flags)
{
    err = pfc_ipcclnt_sess_altcreate5(&_sess, conn, name, service, flags);
}

/*
 * Create a new client session on the given connection handle.
 *
 * This constructor uses std::string to specify IPC service name.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is set to `err'.
 *
 *    Otherwise error number which indicates the cause of error is set
 *    to `err'. If a non-zero value is set to `err', this instance must
 *    be discarded immediately.
 */
inline
ClientSession::ClientSession(pfc_ipcconn_t conn, const std::string &name,
                             pfc_ipcid_t service, int &err)
{
    err = pfc_ipcclnt_sess_altcreate(&_sess, conn, name.c_str(), service);
}

inline
ClientSession::ClientSession(pfc_ipcconn_t conn, const std::string &name,
                             pfc_ipcid_t service, int &err, uint32_t flags)
{
    err = pfc_ipcclnt_sess_altcreate5(&_sess, conn, name.c_str(), service,
                                      flags);
}

/*
 * Create a new client session using the specified pfc_ipcsess_t pointer.
 *
 * This constructor is provided to derive additional data from an IPC
 * event using C++ APIs.
 */
inline
ClientSession::ClientSession(pfc_ipcsess_t *sess)
    : _sess(sess)
{
}

/*
 * Destructor of the IPC client session instance.
 */
inline
ClientSession::~ClientSession()
{
    if (PFC_EXPECT_TRUE(_sess != NULL)) {
        __pfc_ipcclnt_sess_destroy(_sess);
    }
}

/*
 * Reset the session state.
 *
 * `name' and `service' is a pair of IPC service name and ID for new
 * IPC service request. If you want to issue a new IPC service request
 * on existing client session, the session must be reset by this function.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is returned.
 *    Otherwise error number which indicates the cause of error is
 *    returned.
 */
inline int
ClientSession::reset(const char *name, pfc_ipcid_t service)
{
    return pfc_ipcclnt_sess_reset(_sess, name, service);
}

/*
 * Reset the session state.
 *
 * This method uses std::string to specify IPC service name.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is returned.
 *    Otherwise error number which indicates the cause of error is
 *    returned.
 * Remarks:
 *    Response of the previous IPC request from the server is always
 *    discarded.
 */
inline int
ClientSession::reset(const std::string &name, pfc_ipcid_t service)
{
    return pfc_ipcclnt_sess_reset(_sess, name.c_str(), service);
}

/*
 * Set IPC client session timeout by pfc_timespec_t.
 * Specifying NULL to `timeout' means that an infinite timeout should
 * be used on the IPC client session.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is returned.
 *    Otherwise error number which indicates the cause of error is
 *    returned.
 */
inline int
ClientSession::setTimeout(const pfc_timespec_t *timeout)
{
    return pfc_ipcclnt_sess_settimeout(_sess, timeout);
}

/*
 * Issue a request of the IPC service.
 *
 * All PDUs previously added by the call of addOutput() are sent to
 * the IPC server. On successful return, getResponse() will be available
 * to obtain additional response from the IPC server.
 *
 * Calling/Exit State:
 *    Upon successful completion, a response code sent by the IPC server
 *    is set to `response', and zero is returned.
 *
 *    ENOSYS is returned if the IPC server does not implement the IPC
 *    service specified by a pair of the service name and ID, passed to
 *    constructor or reset() method.
 *
 *    Otherwise error number which indicates the cause of error is
 *    returned.
 *
 * Remarks:
 *    PDUs added to the IPC output stream will be discarded on return,
 *    as long as the session is active on another thread.
 */
inline int
ClientSession::invoke(pfc_ipcresp_t &response)
{
    return pfc_ipcclnt_sess_invoke(_sess, &response);
}

/*
 * Cancel ongoing IPC invocation request on this IPC session.
 *
 * Ongoing call of invoke() on this session will get ECANCELED error.
 * If PFC_TRUE is passed to `discard', the state of the client session
 * will be changed to DISCARD. That is, further IPC service request on
 * the given session will get ESHUTDOWN error.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is returned.
 *
 *    EPERM is returned if the this session is not created with
 *    PFC_IPCSSF_CANCELABLE or PFC_IPCSSF_NOGLOBCANCEL flag.
 */
inline int
ClientSession::cancel(pfc_bool_t discard)
{
    return pfc_ipcclnt_sess_cancel(_sess, discard);
}

/*
 * Append additional data received from the IPC server in the specified
 * client session to the output stream of this session.
 *
 * `sess' is another IPC client session which keeps additional data
 * received from the IPC server. Its state must be RESULT.
 *
 * `begin' and `end' specifies additional data in `sess' to be forwarded.
 * `begin' is the inclusive beginning index, and `end' is the exclusive
 * ending index of additional data. For instance, if `begin' is 3 and
 * `end' is 10, additional data in `sess' at the index from 3 to 9 are
 * appended to the IPC output stream in this session. Needless to say,
 * `end' must be greater than `begin'.
 *
 * If the value specified to `end' is greater than the number of
 * additional data in `sess', it is treated as if the number of
 * additional data in `sess' is specified to `end'.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is returned.
 *    Otherwise error number which indicates the cause of error is
 *    returned.
 */
inline int
ClientSession::forward(ClientSession &sess, uint32_t begin, uint32_t end)
{
    return pfc_ipcclnt_forward(_sess, sess._sess, begin, end);
}

/*
 * Append additional data received from the IPC client in the specified
 * IPC server session to the output stream of this session.
 */
inline int
ClientSession::forward(ServerSession &sess, uint32_t begin, uint32_t end)
{
    return pfc_ipcclnt_forward_fromsrv(_sess, sess._srv, begin, end);
}

/*
 * Append additional data received from the IPC server in the this client
 * session to the output stream of the specified IPC server session.
 */
inline int
ClientSession::forwardTo(ServerSession &sess, uint32_t begin, uint32_t end)
{
    return pfc_ipcclnt_forward_tosrv(sess._srv, _sess, begin, end);
}

/*
 * Add a PDU to the output stream.
 * When invoke() method is called, all PDUs added to the IPC client
 * session are sent to the IPC server in added order.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is returned.
 *    Otherwise error number which indicates the cause of error is
 *    returned.
 */

// Signed 8-bit value.
inline int
ClientSession::addOutput(int8_t data)
{
    return pfc_ipcclnt_output_int8(_sess, data);
}

inline int
ClientSession::addOutputInt8(int8_t data)
{
    return addOutput(data);
}

// Unsigned 8-bit value.
inline int
ClientSession::addOutput(uint8_t data)
{
    return pfc_ipcclnt_output_uint8(_sess, data);
}

inline int
ClientSession::addOutputUint8(uint8_t data)
{
    return addOutput(data);
}

// Signed 16-bit value.
inline int
ClientSession::addOutput(int16_t data)
{
    return pfc_ipcclnt_output_int16(_sess, data);
}

inline int
ClientSession::addOutputInt16(int16_t data)
{
    return addOutput(data);
}

// Unsigned 16-bit value.
inline int
ClientSession::addOutput(uint16_t data)
{
    return pfc_ipcclnt_output_uint16(_sess, data);
}

inline int
ClientSession::addOutputUint16(uint16_t data)
{
    return addOutput(data);
}

// Signed 32-bit value.
inline int
ClientSession::addOutput(int32_t data)
{
    return pfc_ipcclnt_output_int32(_sess, data);
}

inline int
ClientSession::addOutputInt32(int32_t data)
{
    return addOutput(data);
}

// Unsigned 32-bit value.
inline int
ClientSession::addOutput(uint32_t data)
{
    return pfc_ipcclnt_output_uint32(_sess, data);
}

inline int
ClientSession::addOutputUint32(uint32_t data)
{
    return addOutput(data);
}

// Signed 64-bit value.
inline int
ClientSession::addOutput(int64_t data)
{
    return pfc_ipcclnt_output_int64(_sess, data);
}

inline int
ClientSession::addOutputInt64(int64_t data)
{
    return addOutput(data);
}

// Unsigned 64-bit value.
inline int
ClientSession::addOutput(uint64_t data)
{
    return pfc_ipcclnt_output_uint64(_sess, data);
}

inline int
ClientSession::addOutputUint64(uint64_t data)
{
    return addOutput(data);
}

// Single precision floating point.
inline int
ClientSession::addOutput(float data)
{
    return pfc_ipcclnt_output_float(_sess, data);
}

inline int
ClientSession::addOutputFloat(float data)
{
    return addOutput(data);
}

// Double precision floating point.
inline int
ClientSession::addOutput(double data)
{
    return pfc_ipcclnt_output_double(_sess, data);
}

inline int
ClientSession::addOutputDouble(double data)
{
    return addOutput(data);
}

// IPv4 address.
inline int
ClientSession::addOutput(struct in_addr &data)
{
    return pfc_ipcclnt_output_ipv4(_sess, &data);
}

// IPv6 address.
inline int
ClientSession::addOutput(struct in6_addr &data)
{
    return pfc_ipcclnt_output_ipv6(_sess, &data);
}

// String. (pointer)
inline int
ClientSession::addOutput(const char *data)
{
    return pfc_ipcclnt_output_string(_sess, data);
}

// String. (std::string)
inline int
ClientSession::addOutput(const std::string &data)
{
    return pfc_ipcclnt_output_string(_sess, data.c_str());
}

// Binary data.
inline int
ClientSession::addOutput(const uint8_t *data, uint32_t length)
{
    return pfc_ipcclnt_output_binary(_sess, data, length);
}

// IPC structure specified by pfc_ipcstdef_t.
inline int
ClientSession::addOutput(const pfc_ipcstdef_t &def, pfc_cptr_t data)
{
    return pfc_ipcclnt_output_stdef(_sess, &def, data);
}

// NULL.
inline int
ClientSession::addOutput(void)
{
    return pfc_ipcclnt_output_null(_sess);
}

/*
 * Get additional response from the IPC server.
 * Additional response is represented by array of PDU. Array index,
 * starts from zero, must be specified to `index'.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is returned.
 *    EINVAL is returned if invalid argument is specified.
 *    EPERM is returned if the PDU type at the given index does not match
 *    the method.
 */

// Signed 8-bit integer.
inline int
ClientSession::getResponse(uint32_t index, int8_t &data)
{
    return pfc_ipcclnt_getres_int8(_sess, index, &data);
}

// Unsigned 8-bit integer.
inline int
ClientSession::getResponse(uint32_t index, uint8_t &data)
{
    return pfc_ipcclnt_getres_uint8(_sess, index, &data);
}

// Signed 16-bit integer.
inline int
ClientSession::getResponse(uint32_t index, int16_t &data)
{
    return pfc_ipcclnt_getres_int16(_sess, index, &data);
}

// Unsigned 16-bit integer.
inline int
ClientSession::getResponse(uint32_t index, uint16_t &data)
{
    return pfc_ipcclnt_getres_uint16(_sess, index, &data);
}

// Signed 32-bit integer.
inline int
ClientSession::getResponse(uint32_t index, int32_t &data)
{
    return pfc_ipcclnt_getres_int32(_sess, index, &data);
}

// Unsigned 32-bit integer.
inline int
ClientSession::getResponse(uint32_t index, uint32_t &data)
{
    return pfc_ipcclnt_getres_uint32(_sess, index, &data);
}

// Signed 64-bit integer.
inline int
ClientSession::getResponse(uint32_t index, int64_t &data)
{
    return pfc_ipcclnt_getres_int64(_sess, index, &data);
}

// Unsigned 64-bit integer.
inline int
ClientSession::getResponse(uint32_t index, uint64_t &data)
{
    return pfc_ipcclnt_getres_uint64(_sess, index, &data);
}

// Single precision floating point.
inline int
ClientSession::getResponse(uint32_t index, float &data)
{
    return pfc_ipcclnt_getres_float(_sess, index, &data);
}

// Double precision floating point.
inline int
ClientSession::getResponse(uint32_t index, double &data)
{
    return pfc_ipcclnt_getres_double(_sess, index, &data);
}

// IPv4 address.
inline int
ClientSession::getResponse(uint32_t index, struct in_addr &data)
{
    return pfc_ipcclnt_getres_ipv4(_sess, index, &data);
}

// IPv6 address.
inline int
ClientSession::getResponse(uint32_t index, struct in6_addr &data)
{
    return pfc_ipcclnt_getres_ipv6(_sess, index, &data);
}

// Pointer to string.
//
// Remarks:
//     - Buffer set to `data' is read-only, and it will be freed when the
//       client session is destroyed, or reset() method is called.
//
//     - String PDU accepts NULL. So NULL may be set to `data' if the
//       client sends NULL as string PDU.
inline int
ClientSession::getResponse(uint32_t index, const char *&data)
{
    return pfc_ipcclnt_getres_string(_sess, index, &data);
}

// Binary data.
// A pointer to binary data is set to `data', and its length in bytes is
// set to `length'.
//
// Remarks:
//     - Buffer set to `data' is read-only, and it will be freed when the
//       client session is destroyed, or reset() method is called.
//
//     - Binary PDU accepts NULL. So NULL may be set to `data' if the
//       client sends NULL as binary PDU.
inline int
ClientSession::getResponse(uint32_t index, const uint8_t *&data,
                           uint32_t &length)
{
    return pfc_ipcclnt_getres_binary(_sess, index, &data, &length);
}

// IPC structure specified by pfc_ipcstdef_t.
inline int
ClientSession::getResponse(uint32_t index, const pfc_ipcstdef_t &def,
                           pfc_ptr_t datap)
{
    return pfc_ipcclnt_getres_stdef(_sess, index, &def, datap);
}

/*
 * Return the number of PDUs in additional response from the IPC server.
 */
inline uint32_t
ClientSession::getResponseCount(void)
{
    return pfc_ipcclnt_getrescount(_sess);
}

/*
 * Determine the PDU type of additional response specified by the PDU
 * array index.
 *
 * Calling/Exit State:
 *    Upon successful completion, PDU data type is stored to `type', and
 *    zero is returned.
 *    Otherwise error number which indicates the cause of error is
 *     returned.
 */
inline int
ClientSession::getResponseType(uint32_t index, pfc_ipctype_t &type)
{
    return pfc_ipcclnt_getrestype(_sess, index, &type);
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
ClientSession::getResponseStructName(uint32_t index, const char *&name)
{
    return pfc_ipcclnt_getres_structname(_sess, index, &name);
}

inline int
ClientSession::getResponseStructName(uint32_t index, std::string &name)
{
    const char  *nm;
    int  err(pfc_ipcclnt_getres_structname(_sess, index, &nm));

    if (PFC_EXPECT_TRUE(err == 0)) {
        name = nm;
    }

    return err;
}

/*
 * Import inlined accessors for user-defined IPC structs.
 */
#define _PFCXX_IPC_IMPORT_STRUCT_CLIENT
#include <pfcxx/ipc_client_inline.hh>
#undef  _PFCXX_IPC_IMPORT_STRUCT_CLIENT

/*
 * Remarks:
 *    IPC event APIs for C++ are provided by libpfcxx_ipcclnt.
 */

class IpcEventAttr;
class IpcEventHandler;
#ifdef  _PFC_LIBPFCXX_IPCCLNT_BUILD
class IpcEventSystem;
#endif  /* _PFC_LIBPFCXX_IPCCLNT_BUILD */

/*
 * IPC event mask which represents set of IPC event types.
 */
class IpcEventMask
{
    friend class  ::pfc::core::ipc::IpcEventAttr;

public:
    /* Create an event mask which contains all IPC event types. */
    IpcEventMask(): _mask(PFC_IPC_EVENT_MASK_FILL) {}

    /* Create an event mask which contains the specified IPC event type. */
    explicit IpcEventMask(pfc_ipcevtype_t type)
        : _mask(PFC_IPC_EVENT_MASK_BIT(type)) {}

    /* Copy constructor. */
    explicit IpcEventMask(const IpcEventMask &mask) : _mask(mask._mask) {}

    /* Clear all bits in the event mask. */
    inline void
    empty(void)
    {
        pfc_ipcevent_mask_empty(&_mask);
    }

    /* Fill all bits in the event mask. */
    inline void
    fill(void)
    {
        pfc_ipcevent_mask_fill(&_mask);
    }

    /*
     * Add the specified IPC event type to the event mask.
     *
     * Calling/Exit State:
     *    Upon successful completion, zero is returned.
     *    EINVAL is returned if the specified event type is invalid.
     */
    inline int
    add(pfc_ipcevtype_t type)
    {
        return pfc_ipcevent_mask_add(&_mask, type);
    }

    /*
     * Remove the specified IPC event type from the event mask.
     *
     * Calling/Exit State:
     *    Upon successful completion, zero is returned.
     *    EINVAL is returned if the specified event type is invalid.
     */
    inline int
    remove(pfc_ipcevtype_t type)
    {
        return pfc_ipcevent_mask_remove(&_mask, type);
    }

    /*
     * Return PFC_TRUE if the event mask contains the specified IPC
     * event type.
     */
    inline pfc_bool_t
    test(pfc_ipcevtype_t type) const
    {
        return pfc_ipcevent_mask_test(&_mask, type);
    }

    /* Return a pointer to the current event mask. */
    inline const pfc_ipcevmask_t *
    getMask(void) const
    {
        return &_mask;
    }

private:
    /* Event mask value. */
    pfc_ipcevmask_t    _mask;
};

/*
 * IPC event handler attributes.
 */
class IpcEventAttr
    : boost::noncopyable
{
#ifdef  _PFC_LIBPFCXX_IPCCLNT_BUILD
    friend class  ::pfc::core::ipc::IpcEventSystem;
#endif  /* _PFC_LIBPFCXX_IPCCLNT_BUILD */

public:
    /*
     * Create an event attributes object with default parameters.
     */
    IpcEventAttr()
    {
        // This function never fails unless NULL is passed.
        (void)pfc_ipcevent_attr_init(&_attr);
    }

    /*
     * Destroy the event attributes object.
     */
    ~IpcEventAttr()
    {
        pfc_ipcevent_attr_destroy(&_attr);
    }

    /*
     * Set a string pointer which represents the name of the IPC host set
     * in this object to `name'.
     * NULL is set to `name' if no host set is set in this object.
     */
    inline void
    getHostSet(const char *&name) const
    {
        // This function never fails unless NULL is passed.
        (void)pfc_ipcevent_attr_gethostset(&_attr, &name);
    }

    /*
     * Set the name of the IPC host set, which determines IPC server to listen
     * IPC events.
     * If `name' is NULL, the IPC host set in this object is reset to initial
     * state.
     *
     * Calling/Exit State:
     *    Upon successful completion, zero is returned.
     *    ENODEV is returned if the IPC host set specified by `name' is not
     *    found.
     */
    inline int
    setHostSet(const char *name)
    {
        return pfc_ipcevent_attr_sethostset(&_attr, name);
    }

    /*
     * Get target event mask associated with the specified IPC service name
     * in this event attributes object.
     *
     * If `service' is NULL, this function searches for the event mask bits
     * associated with IPC channel state change event.
     */
    inline void
    getTarget(const char *service, IpcEventMask &mask) const
    {
        // This function never fails unless NULL is passed.
        (void)pfc_ipcevent_attr_gettarget(&_attr, service, &mask._mask);
    }

    /*
     * Set all target event mask bits associated with the IPC service name
     * specified by `service'.
     *
     * If `service' is NULL, this function sets all event mask bits associated
     * with IPC channel state change event.
     *
     * Calling/Exit State:
     *    Upon successful completion, zero is returned.
     *    Otherwise error number which indicates the cause of error is
     *    returned.
     */
    inline int
    addTarget(const char *service)
    {
        return pfc_ipcevent_attr_addtarget(&_attr, service, NULL);
    }

    /*
     * Add the target event mask specified by `mask' to the event mask
     * associated with the IPC service name specified by `service'.
     *
     * If an event mask is already associated with the service name `service'
     * in this object, the event mask specified by `mask' is merged to
     * existing event mask.
     *
     * If `service' is NULL, this function merges the event mask `mask' to
     * the event mask associated with IPC channel state change event.
     *
     * Calling/Exit State:
     *    Upon successful completion, zero is returned.
     *    Otherwise error number which indicates the cause of error is
     *    returned.
     */
    inline int
    addTarget(const char *service, const IpcEventMask &mask)
    {
        return pfc_ipcevent_attr_addtarget(&_attr, service, mask.getMask());
    }

    /*
     * Reset the target event mask set to initial state, which targets all
     * IPC events, including IPC channel state change events.
     */
    inline void
    resetTarget(void)
    {
        // This function never fails unless NULL is passed.
        (void)pfc_ipcevent_attr_resettarget(&_attr);
    }

    /*
     * Get priority value of the IPC event handler.
     */
    inline void
    getPriority(uint32_t &priority) const
    {
        // This function never fails unless NULL is passed.
        (void)pfc_ipcevent_attr_getpriority(&_attr, &priority);
    }

    /*
     * Set priority value f the IPC event handler.
     * Event handlers are called in ascending priority order.
     */
    inline void
    setPriority(uint32_t priority)
    {
        // This function never fails unless NULL is passed.
        (void)pfc_ipcevent_attr_setpriority(&_attr, priority);
    }

    /*
     * Get IPC event delivery logging configuration.
     * PFC_TRUE is set to `log' if event delivery logging is enabled.
     * Otherwise PFC_FALSE is set to `log'.
     */
    inline void
    getLog(pfc_bool_t &log) const
    {
        // This function never fails unless NULL is passed.
        (void)pfc_ipcevent_attr_getlog(&_attr, &log);
    }

    /*
     * Enable or disable IPC event delivery logging.
     * Delivery logging is enabled if PFC_TRUE is specified.
     * Otherwise delivery logging is disable.
     */
    inline void
    setLog(pfc_bool_t log)
    {
        // This function never fails unless NULL is passed.
        (void)pfc_ipcevent_attr_setlog(&_attr, log);
    }

private:
    /* Event attributes object. */
    pfc_ipcevattr_t   _attr;
};

/*
 * IPC event object.
 */
class IpcEvent
    : boost::noncopyable
{
#ifdef  _PFC_LIBPFCXX_IPCCLNT_BUILD
    friend class  ::pfc::core::ipc::IpcEventSystem;
#endif  /* _PFC_LIBPFCXX_IPCCLNT_BUILD */

public:
    /*
     * Return a serial ID assigned to this event.
     */
    inline pfc_ipcevid_t
    getSerial(void) const
    {
        return pfc_ipcevent_getserial(_event);
    }

    /*
     * Return IPC event type of this event.
     */
    inline pfc_ipcevtype_t
    getType(void) const
    {
        return pfc_ipcevent_gettype(_event);
    }

    /*
     * Set creation time of this event to `ts'.
     */
    inline void
    getTime(pfc_timespec_t &ts) const
    {
        pfc_ipcevent_gettime(_event, &ts);
    }

    /*
     * Return the name of IPC channel which generated this event.
     */
    inline const char *
    getChannelName(void) const
    {
        return pfc_ipcevent_getchannelname(_event);
    }

    /*
     * Return the host address of the IPC server which generated this event.
     */
    inline const pfc_hostaddr_t *
    getHostAddress(void) const
    {
        return pfc_ipcevent_gethostaddr(_event);
    }

    /*
     * Return the IPC service name associated with this event.
     */
    inline const char *
    getServiceName(void) const
    {
        return pfc_ipcevent_getservicename(_event);
    }

    /*
     * Return a pointer to pseudo IPC client session.
     * It can be used to derive additional data from this event.
     */
    inline pfc_ipcsess_t *
    getSession(void) const
    {
        return pfc_ipcevent_getsess(_event);
    }

    /*
     * Determine whether this event is an IPC channel state change event
     * or not.
     *
     * PFC_TRUE is returned if this event is an IPC channel state change
     * event. Otherwise PFC_FALSE is returned.
     */
    inline pfc_bool_t
    isStateChangeEvent(void) const
    {
        return pfc_ipcevent_isstatechange(_event);
    }

private:
    /*
     * Construct an IPC event object.
     */
    explicit IpcEvent(pfc_ipcevent_t *event) : _event(event) {}

    /* IPC event object. */
    pfc_ipcevent_t    *_event;
};

/*
 * Base class of IPC event handler.
 */
class IpcEventHandler
    : boost::noncopyable
{
#ifdef  _PFC_LIBPFCXX_IPCCLNT_BUILD
    friend class  ::pfc::core::ipc::IpcEventSystem;
#endif  /* _PFC_LIBPFCXX_IPCCLNT_BUILD */

public:
    /*
     * Default constructor.
     */
    IpcEventHandler() : _id(PFC_IPCEVHDLR_INVALID) {}

    /*
     * IPC event handler's destructor.
     * Destructor will be called when this event handler is unregistered.
     */
    virtual ~IpcEventHandler();

    /*
     * Return handler's identifier.
     * PFC_IPCEVHDLR_INVALID is returned if this handler is not registered
     * to the IPC event subsystem.
     */
    inline pfc_ipcevhdlr_t
    getId(void) const
    {
        return _id;
    }

    /*
     * Interface of IPC event handler.
     * IPC event object is passed to argument.
     */
    virtual void  eventHandler(const IpcEvent &event) = 0;

    /*
     * Return the name of this handler.
     * Currently, the name of event handler is used only for event delivery
     * logging.
     *
     * This method of this class always returns NULL, which means anonymous
     * handler.
     */
    virtual const char  *getName(void);

private:
    /* Handler's identifier. */
    pfc_ipcevhdlr_t    _id;
};

#ifndef PFC_MODULE_BUILD

/*
 * Prototypes.
 */
extern int  add_event_handler(const char *channel, IpcEventHandler *handler,
                              const IpcEventAttr *attr = NULL);
extern int  remove_event_handler(pfc_ipcevhdlr_t id);

/*
 * static inline int
 * remove_event_handler(IpcEventHandler *handler)
 *      Remove IPC event handler specified by `handler' from the IPC event
 *      subsystem.
 *
 * Calling/Exit State:
 *    Upon successful completion, zero is returned.
 *    Otherwise error number which indicates the cause of error is returned.
 */
static inline int
remove_event_handler(IpcEventHandler *handler)
{
    if (PFC_EXPECT_FALSE(handler == NULL)) {
        return EINVAL;
    }

    return remove_event_handler(handler->getId());
}

#endif  /* !PFC_MODULE_BUILD */

}       // ipc
}       // core
}       // pfc

#endif  /* !_PFCXX_IPC_CLIENT_HH */
