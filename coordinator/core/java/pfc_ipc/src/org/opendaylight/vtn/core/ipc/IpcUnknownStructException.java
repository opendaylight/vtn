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
 *   The {@code IpcUnknownStructException} class is an exception to notify
 *   that an unknown IPC structure has been received via IPC connection.
 * </p>
 *
 * @since	C10
 */
public class IpcUnknownStructException extends IpcException
{
	final static long  serialVersionUID = -333391420717722484L;

	/**
	 * Construct an {@code IpcUnknownStructException} without detailed
	 * message.
	 */
	public IpcUnknownStructException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcUnknownStructException} with the specified
	 * detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcUnknownStructException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcUnknownStructException} with the specified
	 * detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcUnknownStructException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcUnknownStructException} with the specified
	 * cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcUnknownStructException(Throwable cause)
	{
		super(cause);
	}
}
