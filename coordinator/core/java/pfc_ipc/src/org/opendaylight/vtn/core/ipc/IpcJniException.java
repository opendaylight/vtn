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
 *   The {@code IpcJniException} class is an exception to notify that
 *   a JNI interface returned an error.
 * </p>
 *
 * @since	C10
 */
public class IpcJniException extends IpcException
{
	final static long  serialVersionUID = -1662685427386463994L;

	/**
	 * Construct an {@code IpcJniException} without detailed
	 * message.
	 */
	public IpcJniException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcJniException} with the
	 * specified detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcJniException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcJniException} with the
	 * specified detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcJniException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcJniException} with the
	 * specified cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcJniException(Throwable cause)
	{
		super(cause);
	}
}
