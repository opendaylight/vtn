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
 *   listener session has been successfully established.
 * </p>
 *
 * @since	C10
 */
public final class ChannelUpEvent extends ChannelStateChangeEvent
{
	/**
	 * <p>
	 *   Event type of {@code ChannelUpEvent}.
	 * </p>
	 */
	public final static int  TYPE = 0;

	/**
	 * <p>
	 *   Construct a channel up event.
	 * </p>
	 *
	 * @param event		An IPC event handle.
	 * @param serial	Serial number of this event.
	 * @param session	Client session handle in this event.
	 */
	ChannelUpEvent(long event, int serial, long session)
	{
		super(event, serial, TYPE, session);
	}

	/**
	 * <p>
	 *   Return an additional string to be added to the result of
	 *   {@link #toString()}.
	 * </p>
	 *
	 * @return	{@code null} is always returned because this class
	 *		has no additional data.
	 */
	@Override
	synchronized String toStringImpl()
	{
		return null;
	}
}
