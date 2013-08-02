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
 *   The {@code IpcTimedOutException} class is an exception to notify that
 *   a process did not complete until the specified deadline.
 *   Typically <strong>ETIMEDOUT</strong>  error returned by the IPC framework
 *   library is reported by this exception.
 * </p>
 *
 * @since	C10
 */
public class IpcTimedOutException extends IpcException
{
	final static long  serialVersionUID = -1061637227805651584L;

	/**
	 * Construct an {@code IpcTimedOutException} without detailed
	 * message.
	 */
	public IpcTimedOutException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcTimedOutException} with the specified
	 * detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcTimedOutException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcTimedOutException} with the specified
	 * detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcTimedOutException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcTimedOutException} with the specified
	 * cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcTimedOutException(Throwable cause)
	{
		super(cause);
	}
}
