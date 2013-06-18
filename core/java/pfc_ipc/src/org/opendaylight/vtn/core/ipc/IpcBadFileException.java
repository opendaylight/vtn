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
 *   The {@code IpcBadFileException} class is an exception to notify that an
 *   invalid file descriptor has been  used. Typically <strong>EBADF</strong>
 *   error returned by the IPC framework library is reported by this exception.
 * </p>
 *
 * @since	C10
 */
public class IpcBadFileException extends IpcException
{
	final static long  serialVersionUID = 1369590826248180970L;

	/**
	 * Construct an {@code IpcBadFileException} without detailed
	 * message.
	 */
	public IpcBadFileException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcBadFileException} with the specified
	 * detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcBadFileException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcBadFileException} with the specified
	 * detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcBadFileException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcBadFileException} with the specified
	 * cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcBadFileException(Throwable cause)
	{
		super(cause);
	}
}
