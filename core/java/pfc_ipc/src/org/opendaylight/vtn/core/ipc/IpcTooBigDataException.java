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
 *   An {@code IpcTooBigDataException} is thrown when the total size of
 *   the additional data in a {@link ClientSession} object exceeds the
 *   internal limit.
 * </p>
 *
 * @since	C10
 */
public class IpcTooBigDataException extends IpcException
{
	final static long  serialVersionUID = -7521864362650171088L;

	/**
	 * Construct an {@code IpcTooBigDataException} without detailed
	 * message.
	 */
	public IpcTooBigDataException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcTooBigDataException} with the
	 * specified detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcTooBigDataException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcTooBigDataException} with the
	 * specified detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcTooBigDataException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcTooBigDataException} with the
	 * specified cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcTooBigDataException(Throwable cause)
	{
		super(cause);
	}
}
