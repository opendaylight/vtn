/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

/**
 * An instance of {@code ChannelDown} represents the cause of disconnection
 * of the IPC event listener session.
 *
 * @since	C10
 * @see		ChannelDownEvent
 */
public enum ChannelDownCause
{
	/**
	 * This value means the connection has refused by the IPC server.
	 */
	REFUSED,

	/**
	 * This value means the connection reset by the IPC server.
	 */
	RESET,

	/**
	 * This value means the connection has been disconnected by the IPC
	 * server while the IPC client is sending data.
	 */
	HANGUP,

	/**
	 * This value means an I/O transaction on the IPC event listener
	 * session has timed out.
	 */
	TIMEDOUT,

	/**
	 * This value means the IPC server has refused the connection because
	 * of authentication failure.
	 */
	AUTHFAIL,

	/**
	 * This value means the IPC server has refused the connection because
	 * of too many connections.
	 */
	TOOMANY,

	/**
	 * This value means the IPC client has discarded the event listener
	 * session because of an error.
	 */
	ERROR;
}
