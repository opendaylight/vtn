/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import org.opendaylight.vtn.core.util.HostAddress;

/**
 * An instance of {@code ChannelState} represents the state of IPC event
 * listener session.
 *
 * @since	C10
 * @see		IpcEventSystem#getChannelState(String)
 * @see		IpcEventSystem#getChannelState(String, HostAddress)
 */
public enum ChannelState
{
	/**
	 * This value represents that the IPC event listener session is
	 * not established.
	 */
	DOWN,

	/**
	 * This value represents that the IPC event listener session is
	 * established.
	 */
	UP,

	/**
	 * This value represents that the IPC client library is now trying
	 * to establish the IPC event listener session with the IPC server.
	 * The IPC client library will raise an {@link IpcEvent} later, which
	 * represents a channel state change event.
	 */
	IN_PROGRESS;
}
