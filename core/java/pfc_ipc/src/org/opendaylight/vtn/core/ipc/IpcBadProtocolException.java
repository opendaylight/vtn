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
 *   The {@code IpcBadProtocolException} class is an exception to notify
 *   that unexpected data has been received via IPC connection.
 *   Typically <strong>EPROTO</strong> error returned by the IPC framework
 *   library is reported by this exception.
 * </p>
 *
 * @since	C10
 */
public class IpcBadProtocolException extends IpcException
{
	final static long  serialVersionUID = 5528166254031079765L;

	/**
	 * Construct an {@code IpcBadProtocolException} without detailed
	 * message.
	 */
	public IpcBadProtocolException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcBadProtocolException} with the
	 * specified detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcBadProtocolException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcBadProtocolException} with the
	 * specified detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcBadProtocolException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcBadProtocolException} with the
	 * specified cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcBadProtocolException(Throwable cause)
	{
		super(cause);
	}
}
