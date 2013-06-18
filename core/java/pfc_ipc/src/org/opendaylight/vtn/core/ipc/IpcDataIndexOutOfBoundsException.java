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
 *   The {@code IpcDataIndexOutOfBoundsException} class is an exception to
 *   notify that the additional data has been accessed with an illegal index.
 * </p>
 *
 * @since	C10
 */
public class IpcDataIndexOutOfBoundsException extends IpcException
{
	final static long  serialVersionUID = 6354025617692983035L;

	/**
	 * Construct an {@code IpcDataIndexOutOfBoundsException} without
	 * detailed message.
	 */
	public IpcDataIndexOutOfBoundsException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcDataIndexOutOfBoundsException} with the
	 * specified detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcDataIndexOutOfBoundsException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcDataIndexOutOfBoundsException} with the
	 * specified detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcDataIndexOutOfBoundsException(String message,
						Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcDataIndexOutOfBoundsException} with the
	 * specified cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcDataIndexOutOfBoundsException(Throwable cause)
	{
		super(cause);
	}
}
