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
 *   The {@code IpcConnectionResetException} class is an exception to notify
 *   that a connection has been reset by peer.
 *   Typically <strong>ECONNRESET</strong> error returned by the IPC framework
 *   library is reported by this exception.
 * </p>
 *
 * @since	C10
 */
public class IpcConnectionResetException extends IpcException
{
	final static long  serialVersionUID = 5342350653223791266L;

	/**
	 * Construct an {@code IpcConnectionResetException} without detailed
	 * message.
	 */
	public IpcConnectionResetException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcConnectionResetException} with the
	 * specified detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcConnectionResetException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcConnectionResetException} with the
	 * specified detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcConnectionResetException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcConnectionResetException} with the
	 * specified cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcConnectionResetException(Throwable cause)
	{
		super(cause);
	}
}
