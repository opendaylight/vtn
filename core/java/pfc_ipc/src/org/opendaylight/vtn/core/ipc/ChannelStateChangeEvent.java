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
 * Base class of {@link IpcEvent} implementation which represents a
 * channel state change event.
 *
 * @since	C10
 */
public abstract class ChannelStateChangeEvent extends IpcEvent
{
	/**
	 * Construct a channel state change event.
	 *
	 * @param event		An IPC event handle.
	 * @param serial	Serial number of this event.
	 * @param type		An IPC event type.
	 * @param session	Client session handle in this event.
	 */
	ChannelStateChangeEvent(long event, int serial, int type, long session)
	{
		super(event, serial, type, session);
	}

	/**
	 * <p>
	 *   Determine whether this event is a
	 *   <a href="#chstate-event">channel state change event</a> or not.
	 * </p>
	 * <p>
	 *   This method always returns {@code true} because this class
	 *   represents a channel state change event.
	 * </p>
	 *
	 * @return	{@code true} is always returned.
	 * @throws IllegalStateException
	 *	This event is already invalidated.
	 */
	@Override
	public final boolean isStateChange()
	{
		check();

		return true;
	}
}
