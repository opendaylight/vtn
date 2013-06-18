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
 *   The {@code IpcBrokenPipeException} class is an exception to notify that
 *   a connection has been reset by peer while sending data.
 *   Typically <strong>EPIPE</strong> error returned by the IPC framework
 *   library is reported by this exception.
 * </p>
 *
 * @since	C10
 */
public class IpcBrokenPipeException extends IpcException
{
	final static long  serialVersionUID = 8202284924176909778L;

	/**
	 * Construct an {@code IpcBrokenPipeException} without detailed
	 * message.
	 */
	public IpcBrokenPipeException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcBrokenPipeException} with the
	 * specified detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcBrokenPipeException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcBrokenPipeException} with the
	 * specified detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcBrokenPipeException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcBrokenPipeException} with the
	 * specified cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcBrokenPipeException(Throwable cause)
	{
		super(cause);
	}
}
