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
 *   The {@code IpcUnknownServiceException} class is an exception to notify
 *   that an IPC service invoked by an IPC client is not implemented by
 *   IPC server.
 * </p>
 *
 * @since	C10
 */
public class IpcUnknownServiceException extends IpcException
{
	final static long  serialVersionUID = 2778615439920970281L;

	/**
	 * Construct an {@code IpcUnknownServiceException} without detailed
	 * message.
	 */
	public IpcUnknownServiceException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcUnknownServiceException} with the
	 * specified detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcUnknownServiceException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcUnknownServiceException} with the
	 * specified detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcUnknownServiceException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcUnknownServiceException} with the
	 * specified cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcUnknownServiceException(Throwable cause)
	{
		super(cause);
	}
}
