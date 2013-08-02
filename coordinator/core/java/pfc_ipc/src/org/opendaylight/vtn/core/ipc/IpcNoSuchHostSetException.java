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
 *   The {@code IpcNoSuchHostSetException} class is an exception to notify
 *   that the specified IPC host set does not exist.
 * </p>
 *
 * @since	C10
 * @see		IpcHostSet
 */
public class IpcNoSuchHostSetException extends IpcException
{
	final static long  serialVersionUID = -4798012666395254164L;

	/**
	 * Construct an {@code IpcNoSuchHostSetException} without detailed
	 * message.
	 */
	public IpcNoSuchHostSetException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcNoSuchHostSetException} with the specified
	 * detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcNoSuchHostSetException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcNoSuchHostSetException} with the specified
	 * detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcNoSuchHostSetException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcNoSuchHostSetException} with the specified
	 * cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcNoSuchHostSetException(Throwable cause)
	{
		super(cause);
	}
}
