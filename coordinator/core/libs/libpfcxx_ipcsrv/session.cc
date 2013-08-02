/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * session.cc - IPC server session APIs for C++ language.
 */

#include <pfc/atomic.h>
#include <pfcxx/ipc_server.hh>
#include <ipcsrv_impl.h>

namespace pfc {
namespace core {
namespace ipc {

#define SRVXX_CB_PTR(ptr)       (reinterpret_cast<ServerCallback *>(ptr))

/*
 * Manager class of IPC server session callback.
 */
class ServerCallbackManager
{
public:
    /*
     * static inline void
     * hold(ServerCallback *cbp)
     *      Hold the callback instance specified by `cbp'.
     */
    static inline void
    hold(ServerCallback *cbp)
    {
        pfc_atomic_inc_uint32(&cbp->_refcnt);
    }

    /*
     * static inline void
     * release(ServerCallback *cbp)
     *      Release reference to the callback instance specified by `cbp'.
     *      `cbp' will be deleted if it is no longer used.
     */
    static inline void
    release(ServerCallback *cbp)
    {
        uint32_t  refcnt(pfc_atomic_dec_uint32_old(&cbp->_refcnt));
        if (refcnt == 1) {
            delete cbp;
        }
        else {
            PFC_ASSERT(refcnt != 0);
        }
    }

    static void  invoke(pfc_ipcsrv_t *srv, pfc_ipcsrvcb_type_t type,
                        pfc_ptr_t arg);
    static void  destroy(pfc_ptr_t arg);
};

/*
 * Wrapper function for server callback.
 */
static const pfc_ipcsrvcb_t  ipcsrvxx_cb = {
    &ServerCallbackManager::invoke,   // isc_callback
    &ServerCallbackManager::destroy,  // isc_argdtor
};

/*
 * void PFC_ATTR_HIDDEN
 * ServerCallbackManager::invoke(pfc_ipcsrv_t *srv PFC_ATTR_UNUSED,
 *                               pfc_ipcsrvcb_type_t type, pfc_ptr_t arg)
 *      Invoke callback function.
 *      `arg' must be a pointer to ServerCallback.
 */
void PFC_ATTR_HIDDEN
ServerCallbackManager::invoke(pfc_ipcsrv_t *srv PFC_ATTR_UNUSED,
                        pfc_ipcsrvcb_type_t type, pfc_ptr_t arg)
{
    ServerCallback  *cbp(SRVXX_CB_PTR(arg));

    cbp->callback(type);
}

/*
 * void PFC_ATTR_HIDDEN
 * ServerCallbackManager::destroy(pfc_ptr_t arg)
 *      Destructor of callback argument.
 *      `arg' must be a pointer to ServerCallback.
 */
void PFC_ATTR_HIDDEN
ServerCallbackManager::destroy(pfc_ptr_t arg)
{
    ServerCallback  *cbp(SRVXX_CB_PTR(arg));

    release(cbp);
}

/*
 * ServerSession::~ServerSession()
 *      Destructor of ServerSession.
 */
ServerSession::~ServerSession()
{
    if (_srv != NULL) {
        // Ensure that any callback is not installed.
        clearCallbacks();
    }
}

/*
 * int
 * ServerSession::setCallback(pfc_ipcsrvcb_type_t type, ServerCallback *cbp)
 *      Set IPC server session callback to the server session.
 *
 *      `type' is a callback type defined by pfc_ipcsrvcb_type_t.
 *
 *      `cbp' is a pointer to ServerCallback instance.
 *      It must be a pointer created by `new' operator. It is destroyed by
 *      `delete' operator when the last reference to it is unlinked.
 *
 * Calling/Exit State:
 *      Upon successful completion, zero is returned.
 *      Otherwise error number which indicates the cause of error is returned.
 *
 * Remarks:
 *      - This function must be called by IPC service handler.
 *
 *      - Installed callback is invalidated when IPC service handler
 *        is returned.
 *
 *      - On error return, reference to `cbp' is always unlinked. That is,
 *        `cbp' will be deleted if it is not registered as another callback.
 */
int
ServerSession::setCallback(pfc_ipcsrvcb_type_t type, ServerCallback *cbp)
{
    if (PFC_EXPECT_FALSE(cbp == NULL)) {
        return EINVAL;
    }

    // Hold the callback instance in advance.
    ServerCallbackManager::hold(cbp);

    ServerSession &sess(cbp->getSession());
    if (PFC_EXPECT_FALSE(&sess != this)) {
        // Server session does not match.
        ServerCallbackManager::release(cbp);

        return EINVAL;
    }

    return pfc_ipcsrv_setcallback(_srv, type, &ipcsrvxx_cb, cbp);
}

/*
 * ServerCallback::~ServerCallback()
 *      Destructor of ServerCallback.
 */
ServerCallback::~ServerCallback()
{
    PFC_ASSERT(_refcnt == 0);
}

} // ipc
} // core
} // pfc
