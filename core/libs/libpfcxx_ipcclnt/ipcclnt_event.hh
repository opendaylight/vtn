/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFCXX_IPCCLNT_IPCCLNT_EVENT_H
#define	_PFC_LIBPFCXX_IPCCLNT_IPCCLNT_EVENT_H

/*
 * Internal definitions for C++ IPC event subsystem.
 */

#include <pfcxx/ipc_client.hh>
#include <ipcclnt_event.h>

namespace pfc {
namespace core {
namespace ipc {

#define PFC_IPCEVATTR_PTR(attr) (reinterpret_cast<pfc_ipcevattr_t *>(attr))

/*
 * Prototypes.
 */
extern void  setup_wrapper(pfc_ipcevfunc_t &wrapper, ipc_evattr_t &newattr,
                           IpcEventHandler *handler, const IpcEventAttr *attr);
extern pfc_ipcevhdlr_t  *get_id_pointer(IpcEventHandler *handler);

}	// ipc
}	// core
}	// pfc

#endif	/* !_PFC_LIBPFCXX_IPCCLNT_IPCCLNT_EVENT_H */
