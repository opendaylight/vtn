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
 *   An implementation of {@link IpcEvent} which represents a state
 *   notification event.
 * </p>
 * <p>
 *   This event indicates that a channel state notification event has been
 *   requested explicitly by {@link IpcEventSystem#notifyState(int)}.
 *   The state of the IPC event listener session can be obtained by
 *   {@link #getState()}.
 * </p>
 *
 * @since	C10
 */
public final class ChannelStateEvent extends ChannelStateChangeEvent
{
	/**
	 * Event type of {@code ChannelStateEvent}.
	 */
	public final static int  TYPE = 2;

	/**
	 * The state of the IPC event listener session.
	 */
	private final ChannelState  _state;

	/**
	 * Construct a channel up event.
	 *
	 * @param event		An IPC event handle.
	 * @param serial	Serial number of this event.
	 * @param session	Client session handle in this event.
	 * @param up		{@code true} if the channel is up.
	 *			Otherwise {@code false}.
	 */
	ChannelStateEvent(long event, int serial, long session, boolean up)
	{
		super(event, serial, TYPE, session);

		_state = (up) ? ChannelState.UP : ChannelState.DOWN;
	}

	/**
	 * <p>
	 *   Return the state of the IPC event listener session.
	 * </p>
	 * <p>
	 *   {@link ChannelState#UP} is returned if the IPC event listener
	 *   session is established. Otherwise {@link ChannelState#DOWN} is
	 *   returned. This method never returns
	 *   {@link ChannelState#IN_PROGRESS}.
	 * </p>
	 *
	 * @return	The state of the IPC event listener session.
	 * @throws IllegalStateException
	 *	This event is already invalidated.
	 */
	public ChannelState getState()
	{
		check();

		return _state;
	}

	/**
	 * Return an additional string to be added to the result of
	 * {@link #toString()}.
	 *
	 * @return	A string to be added to the result of
	 *		{@link #toString()}.
	 */
	@Override
	synchronized String toStringImpl()
	{
		return " state=" + _state;
	}
}
