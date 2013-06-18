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
 *   An {@code IpcShutdownException} will be thrown when a resource managed
 *   by IPC framework has been removed or invalidated while processing.
 *   Typically <strong>ESHUTDOWN</strong> error returned by the IPC framework
 *   library is reported by this exception.
 * </p>
 *
 * @since	C10
 */
public class IpcShutdownException extends IpcException
{
	final static long  serialVersionUID = 4706865610339822670L;

	/**
	 * Construct an {@code IpcShutdownException} without detailed
	 * message.
	 */
	public IpcShutdownException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcShutdownException} with the specified
	 * detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcShutdownException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcShutdownException} with the specified
	 * detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcShutdownException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcShutdownException} with the specified
	 * cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcShutdownException(Throwable cause)
	{
		super(cause);
	}
}
