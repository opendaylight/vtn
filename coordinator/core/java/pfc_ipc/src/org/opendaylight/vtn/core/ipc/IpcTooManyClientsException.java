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
 *   The {@code IpcTooManyClientsException} class is an exception to notify
 *   that an IPC server has refused a connection from IPC client because of
 *   too many connections.
 * </p>
 *
 * @since	C10
 */
public class IpcTooManyClientsException extends IpcException
{
	final static long  serialVersionUID = -4473836845218399645L;

	/**
	 * Construct an {@code IpcTooManyClientsException} without detailed
	 * message.
	 */
	public IpcTooManyClientsException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcTooManyClientsException} with the
	 * specified detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcTooManyClientsException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcTooManyClientsException} with the
	 * specified detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcTooManyClientsException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcTooManyClientsException} with the
	 * specified cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcTooManyClientsException(Throwable cause)
	{
		super(cause);
	}
}
