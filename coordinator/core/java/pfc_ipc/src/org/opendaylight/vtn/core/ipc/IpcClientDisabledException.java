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
 *   The {@code IpcClientDisabledException} class is an exception to notify
 *   that the IPC client library is already disabled permanently.
 * </p>
 * <p>
 *   If this exception is thrown, any further IPC service request can not
 *   be issued.
 * </p>
 *
 * @since	C10
 * @see		ClientLibrary#disable()
 */
public class IpcClientDisabledException extends IpcException
{
	final static long  serialVersionUID = 2635804609652643357L;

	/**
	 * Construct an {@code IpcClientDisabledException} without detailed
	 * message.
	 */
	public IpcClientDisabledException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcClientDisabledException} with the specified
	 * detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcClientDisabledException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcClientDisabledException} with the specified
	 * detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcClientDisabledException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcClientDisabledException} with the specified
	 * cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcClientDisabledException(Throwable cause)
	{
		super(cause);
	}
}
