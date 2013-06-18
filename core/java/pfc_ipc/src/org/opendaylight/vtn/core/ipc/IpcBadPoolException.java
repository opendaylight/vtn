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
 *   The {@code IpcBadPoolException} class is an exception to notify
 *   that the specified connection pool does not exist.
 * </p>
 *
 * @since	C10
 * @see		ConnectionPool
 */
public class IpcBadPoolException extends IpcException
{
	final static long  serialVersionUID = 7684041795425757873L;

	/**
	 * Construct an {@code IpcBadPoolException} without detailed
	 * message.
	 */
	public IpcBadPoolException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcBadPoolException} with the specified
	 * detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcBadPoolException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcBadPoolException} with the specified
	 * detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcBadPoolException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcBadPoolException} with the specified
	 * cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcBadPoolException(Throwable cause)
	{
		super(cause);
	}
}
