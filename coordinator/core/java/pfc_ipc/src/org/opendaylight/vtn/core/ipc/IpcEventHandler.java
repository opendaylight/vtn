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
 *   The {@code IpcEventHandler} interface defines an interface to be
 *   implemented by IPC event handler.
 * </p>
 *
 * <h4>Remarks</h4>
 * <ul>
 *   <li>
 *     Note that {@link #eventReceived(IpcEvent)} is always invoked on a
 *     single global daemon thread. In other words, the IPC event delivery is
 *     blocked until {@link #eventReceived(IpcEvent)} is returned.
 *     So it should return as soon as possible.
 *   </li>
 *   <li>
 *     An {@link IpcEvent} instance passed to {@link #eventReceived(IpcEvent)}
 *     will be invalidated after it returns. Any necessary data in an
 *     {@link IpcEvent} instance must be preserved in the call of
 *     {@link #eventReceived(IpcEvent)}.
 *   </li>
 * </ul>
 *
 * @since	C10
 * @see		IpcEvent
 */
public interface IpcEventHandler
{
	/**
	 * <p>
	 *   Invoked when an IPC event has been received.
	 * </p>
	 * <p>
	 *   If {@code event} is a channel state change event, an instance
	 *   of specialized class is passed to {@code event}.
	 * </p>
	 * <ul>
	 *   <li>
	 *     An instance of {@link ChannelUpEvent} is passed to {@code event}
	 *     when the IPC event listener session has been successfully
	 *     established.
	 *   </li>
	 * </ul>
	 *
	 * @param event		A received IPC event.
	 */
	public void eventReceived(IpcEvent event);
}
