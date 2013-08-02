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
 *   The {@code IpcException} class is a base class of exception classes
 *   defined by IPC framework. All exception classes provided by IPC framework
 *   inherit this class.
 * </p>
 *
 * @since	C10
 */
public class IpcException extends Exception
{
	final static long  serialVersionUID = 9026078239124130186L;

	/**
	 * Construct an {@code IpcException} without detailed message.
	 */
	public IpcException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcException} with the specified detailed
	 * message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcException} with the specified detailed
	 * message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcException} with the specified cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcException(Throwable cause)
	{
		super(cause);
	}
}
