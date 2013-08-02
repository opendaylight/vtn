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
 *   The {@code IpcResourceBusyException} class is an exception to notify that
 *   the specified IPC resource is busy. Typically <strong>EBUSY</strong>
 *   error returned by the IPC framework library is reported by this exception.
 * </p>
 *
 * @since	C10
 */
public class IpcResourceBusyException extends IpcException
{
	final static long  serialVersionUID = -6156530246319423728L;

	/**
	 * Construct an {@code IpcResourceBusyException} without detailed
	 * message.
	 */
	public IpcResourceBusyException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcResourceBusyException} with the specified
	 * detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcResourceBusyException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcResourceBusyException} with the specified
	 * detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcResourceBusyException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcResourceBusyException} with the specified
	 * cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcResourceBusyException(Throwable cause)
	{
		super(cause);
	}
}
