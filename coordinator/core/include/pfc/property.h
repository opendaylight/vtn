/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

#ifndef	_PFC_PROPERTY_H
#define	_PFC_PROPERTY_H

/*
 * Definitions for PFC daemon system property APIs.
 */

#include <pfc/base.h>

PFC_C_BEGIN_DECL

/*
 * Property keys defined by the system.
 */

/* Absolute path to working directory of the PFC daemon. */
#define	PFC_PROP_WORKDIR		"core.workdir"

/* IPC channel name provided by the PFC daemon. */
#define	PFC_PROP_IPC_CHANNEL		"core.ipc.channel"

/* IPC service for service-ready notification. */
#define	PFC_PROP_IPC_NOTIFY		"core.ipc.notify"

/*
 * Prototypes.
 */
extern const char	*pfc_prop_get(const char *key);

PFC_C_END_DECL

#endif	/* !_PFC_PROPERTY_H */
