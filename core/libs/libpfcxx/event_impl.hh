/*
 * Copyright (c) 2010-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_LIBPFCXX_EVENT_IMPL_H
#define	_PFC_LIBPFCXX_EVENT_IMPL_H

/*
 * Internal definitions for C++ event delivery system.
 */

#include <pfc/base.h>
#include <pfc/event.h>

namespace pfc {
namespace core {

/*
 * Prototypes.
 */
extern void	event_cxx_handler(pfc_event_t event, pfc_ptr_t arg);

}	// core
}	// pfc

#endif	/* !_PFC_LIBPFCXX_EVENT_IMPL_H */
