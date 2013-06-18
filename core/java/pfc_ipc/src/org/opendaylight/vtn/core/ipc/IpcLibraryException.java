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
 *   The {@code IpcLibraryException} class is an exception which represents
 *   an error reported by IPC framework native library.
 *   An error number returned by IPC library can be derived by
 *   {@link #getErrorNumber()}.
 * </p>
 *
 * @since	C10
 */
public class IpcLibraryException extends IpcException
{
	final static long  serialVersionUID = 4643605226697871348L;

	/**
	 * An error number returned by IPC library.
	 */
	private final int  _error;

	/**
	 * <p>
	 *   Construct an {@code IpcLibraryException} without detailed message.
	 * </p>
	 * <p>
	 *   This constructor creates a common message which shows the
	 *   specified error number.
	 * </p>
	 *
	 * @param err	An error number returned by IPC library.
	 */
	public IpcLibraryException(int err)
	{
		super("IPC library error: " + err);
		_error = err;
	}

	/**
	 * Construct an {@code IpcLibraryException} with the specified detailed
	 * message.
	 *
	 * @param err	An error number returned by IPC library.
	 * @param message	The detailed message.
	 */
	public IpcLibraryException(int err, String message)
	{
		super(message);
		_error = err;
	}

	/**
	 * Construct an {@code IpcLibraryException} with the specified detailed
	 * message and cause.
	 *
	 * @param err	An error number returned by IPC library.
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcLibraryException(int err, String message, Throwable cause)
	{
		super(message, cause);
		_error = err;
	}

	/**
	 * Construct an {@code IpcLibraryException} with the specified cause.
	 *
	 * @param err	An error number returned by IPC library.
	 * @param cause		The cause of this exception.
	 */
	public IpcLibraryException(int err, Throwable cause)
	{
		super(cause);
		_error = err;
	}

	/**
	 * Return an error number reported by IPC library.
	 *
	 * @return	A non-zero error number.
	 */
	public int getErrorNumber()
	{
		return _error;
	}
}
