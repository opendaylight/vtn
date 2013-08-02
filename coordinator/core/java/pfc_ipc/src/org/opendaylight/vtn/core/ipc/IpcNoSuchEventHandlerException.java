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
 *   The {@code IpcNoSuchEventHandlerException} class is an exception to
 *   notify that the specified IPC event handler does not exist.
 * </p>
 *
 * @since	C10
 */
public class IpcNoSuchEventHandlerException extends IpcException
{
	final static long  serialVersionUID = 7307494820323204764L;

	/**
	 * Construct an {@code IpcNoSuchEventHandlerException} without detailed
	 * message.
	 */
	public IpcNoSuchEventHandlerException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcNoSuchEventHandlerException} with the
	 * specified detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcNoSuchEventHandlerException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcNoSuchEventHandlerException} with the
	 * specified detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcNoSuchEventHandlerException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcNoSuchEventHandlerException} with the
	 * specified cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcNoSuchEventHandlerException(Throwable cause)
	{
		super(cause);
	}
}
