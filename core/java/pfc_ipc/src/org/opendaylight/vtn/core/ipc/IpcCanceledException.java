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
 *   An {@code IpcCanceledException} will be thrown when an ongoing process
 *   has been canceled by user or system. Typically <strong>ECANCELED</strong>
 *   error returned by the IPC framework library is reported by this exception.
 * </p>
 *
 * @since	C10
 */
public class IpcCanceledException extends IpcException
{
	final static long  serialVersionUID = 8347850808383273065L;

	/**
	 * Construct an {@code IpcCanceledException} without detailed
	 * message.
	 */
	public IpcCanceledException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcCanceledException} with the specified
	 * detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcCanceledException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcCanceledException} with the specified
	 * detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcCanceledException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcCanceledException} with the specified
	 * cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcCanceledException(Throwable cause)
	{
		super(cause);
	}
}
