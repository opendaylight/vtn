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
 * <p>
 *   An implementation of {@link IpcEvent} which notifies that the IPC event
 *   listener session has been disconnected.
 * </p>
 * <p>
 *   The cause of disconnection, which is represented by
 *   {@link ChannelDownCause}, can be obtained by {@link #getCause()}.
 * </p>
 * <dl>
 *   <dt>{@link ChannelDownCause#REFUSED}</dt>
 *   <dd>
 *     The connection has refused by the IPC server.
 *     Typically this event is generated when the IPC server is not running.
 *   </dd>
 *
 *   <dt>{@link ChannelDownCause#RESET}</dt>
 *   <dd>
 *     The connection has been disconnected by the IPC server when the IPC
 *     client tries to receive data.
 *   </dd>
 *
 *   <dt>{@link ChannelDownCause#HANGUP}</dt>
 *   <dd>
 *     The connection has been disconnected by the IPC server when the IPC
 *     client tries to send data.
 *   </dd>
 *
 *   <dt>{@link ChannelDownCause#TIMEDOUT}</dt>
 *   <dd>
 *     An I/O transaction on the IPC event listener session has timed out.
 *   </dd>
 *
 *   <dt>{@link ChannelDownCause#AUTHFAIL}</dt>
 *   <dd>
 *     The IPC server has refused the connection because of authentication
 *     failure.
 *   </dd>
 *
 *   <dt>{@link ChannelDownCause#TOOMANY}</dt>
 *   <dd>
 *     The IPC server has refused the connection because of too many
 *     connections.
 *   </dd>
 *
 *   <dt>{@link ChannelDownCause#ERROR}</dt>
 *   <dd>
 *     The IPC client has discarded the event listener session because
 *     of an error.
 *   </dd>
 * </dl>
 *
 * @since	C10
 */
public final class ChannelDownEvent extends ChannelStateChangeEvent
{
	/**
	 * <p>
	 *   Event type of {@code ChannelDownEvent}.
	 * </p>
	 */
	public final static int  TYPE = 1;

	/**
	 * <p>
	 *   The cause of disconnection.
	 * </p>
	 */
	private final ChannelDownCause  _cause;

	/**
	 * <p>
	 *   Construct a channel down event.
	 * </p>
	 *
	 * @param event		An IPC event handle.
	 * @param serial	Serial number of this event.
	 * @param session	Client session handle in this event.
	 * @param cause		The cause of disconnection.
	 */
	ChannelDownEvent(long event, int serial, long session, int cause)
	{
		super(event, serial, TYPE, session);

		_cause = ChannelDownCause.values()[cause - 1];
	}

	/**
	 * <p>
	 *   Return the cause of disconnection.
	 * </p>
	 *
	 * @return	The cause of disconnection.
	 * @throws IllegalStateException
	 *	This event is already invalidated.
	 */
	public ChannelDownCause getCause()
	{
		check();

		return _cause;
	}

	/**
	 * <p>
	 *   Return an additional string to be added to the result of
	 *   {@link #toString()}.
	 * </p>
	 *
	 * @return	A string to be added to the result of
	 *		{@link #toString()}.
	 */
	@Override
	synchronized String toStringImpl()
	{
		return " cause=" + _cause;
	}
}
