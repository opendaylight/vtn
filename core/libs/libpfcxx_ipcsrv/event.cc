/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

/*
 * event.cc - IPC event APIs for IPC server written in C++ language.
 */

#include <pfcxx/ipc_server.hh>

namespace pfc {
namespace core {
namespace ipc {

/*
 * ServerEvent::~ServerEvent()
 *      Destroy IPC event creation context.
 */
ServerEvent::~ServerEvent()
{
    if (PFC_EXPECT_FALSE(_srv != NULL)) {
        (void)pfc_ipcsrv_event_destroy(_srv);
        _srv = NULL;
    }
}

} // ipc
} // core
} // pfc
