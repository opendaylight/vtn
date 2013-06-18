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
 *   The {@code IpcPermissionDeniedException} class is an exception which
 *   represents permission violation. Typically <strong>EPERM</strong> error
 *   returned by the IPC framework library is reported by this exception.
 * </p>
 *
 * @since	C10
 */
public class IpcPermissionDeniedException extends IpcException
{
	final static long  serialVersionUID = -2429151653464900372L;

	/**
	 * Construct an {@code IpcPermissionDeniedException} without detailed
	 * message.
	 */
	public IpcPermissionDeniedException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcPermissionDeniedException} with the specified
	 * detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcPermissionDeniedException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcPermissionDeniedException} with the specified
	 * detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcPermissionDeniedException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcPermissionDeniedException} with the specified
	 * cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcPermissionDeniedException(Throwable cause)
	{
		super(cause);
	}
}
