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
 *   The {@code IpcBadConnectionException} class is an exception to notify
 *   that the specified connection does not exist.
 * </p>
 *
 * @since	C10
 * @see		IpcConnection
 */
public class IpcBadConnectionException extends IpcException
{
	final static long  serialVersionUID = 9094750227323677896L;

	/**
	 * Construct an {@code IpcBadConnectionException} without detailed
	 * message.
	 */
	public IpcBadConnectionException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcBadConnectionException} with the specified
	 * detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcBadConnectionException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcBadConnectionException} with the specified
	 * detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcBadConnectionException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcBadConnectionException} with the specified
	 * cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcBadConnectionException(Throwable cause)
	{
		super(cause);
	}
}
