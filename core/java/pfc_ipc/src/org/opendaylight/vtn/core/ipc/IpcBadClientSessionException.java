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
 *   The {@code IpcBadClientSessionException} class is an exception to notify
 *   that the specified IPC client session is already destroyed.
 * </p>
 *
 * @since	C10
 * @see		ClientSession
 */
public class IpcBadClientSessionException extends IpcException
{
	final static long  serialVersionUID = 1487113233559579762L;

	/**
	 * Construct an {@code IpcBadClientSessionException} without detailed
	 * message.
	 */
	public IpcBadClientSessionException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcBadClientSessionException} with the specified
	 * detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcBadClientSessionException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcBadClientSessionException} with the specified
	 * detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcBadClientSessionException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcBadClientSessionException} with the specified
	 * cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcBadClientSessionException(Throwable cause)
	{
		super(cause);
	}
}
