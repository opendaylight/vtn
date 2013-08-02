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
 *   The {@code IpcConnectionRefusedException} class is an exception to notify
 *   that an IPC server has refused a connection from IPC client.
 *   Typically <strong>ECONNREFUSED</strong> error returned by the IPC
 *   framework library is reported by this exception.
 * </p>
 *
 * @since	C10
 */
public class IpcConnectionRefusedException extends IpcException
{
	final static long  serialVersionUID = -4267480765058811095L;

	/**
	 * Construct an {@code IpcConnectionRefusedException} without detailed
	 * message.
	 */
	public IpcConnectionRefusedException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcConnectionRefusedException} with the
	 * specified detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcConnectionRefusedException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcConnectionRefusedException} with the
	 * specified detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcConnectionRefusedException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcConnectionRefusedException} with the
	 * specified cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcConnectionRefusedException(Throwable cause)
	{
		super(cause);
	}
}
