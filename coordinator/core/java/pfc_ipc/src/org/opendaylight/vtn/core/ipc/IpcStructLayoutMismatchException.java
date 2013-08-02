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
 *   The {@code IpcStructLayoutMismatchException} class is an exception to
 *   notify that the layout of the received IPC structure does not match the
 *   locally-defined layout.
 * </p>
 *
 * @since	C10
 */
public class IpcStructLayoutMismatchException extends IpcException
{
	final static long  serialVersionUID = 4639075487568648031L;

	/**
	 * Construct an {@code IpcStructLayoutMismatchException} without
	 * detailed message.
	 */
	public IpcStructLayoutMismatchException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcStructLayoutMismatchException} with the
	 * specified detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcStructLayoutMismatchException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcStructLayoutMismatchException} with the
	 * specified detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcStructLayoutMismatchException(String message,
						Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcStructLayoutMismatchException} with the
	 * specified cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcStructLayoutMismatchException(Throwable cause)
	{
		super(cause);
	}
}
