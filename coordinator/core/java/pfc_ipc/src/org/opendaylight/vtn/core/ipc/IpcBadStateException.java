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
 *   The {@code IpcBadStateException} class is an exception to notify that
 *   the state of IPC client session was illegal.
 * </p>
 *
 * @since	C10
 * @see		ClientSession
 */
public class IpcBadStateException extends IpcException
{
	final static long  serialVersionUID = -7929394173671086090L;

	/**
	 * Construct an {@code IpcBadStateException} without detailed
	 * message.
	 */
	public IpcBadStateException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcBadStateException} with the
	 * specified detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcBadStateException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcBadStateException} with the
	 * specified detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcBadStateException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcBadStateException} with the
	 * specified cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcBadStateException(Throwable cause)
	{
		super(cause);
	}
}
