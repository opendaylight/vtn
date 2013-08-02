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
 *   The {@code IpcEventSystemNotReadyException} class is an exception to
 *   notify that the IPC event subsystem in the IPC client library is not yet
 *   initialized.
 * </p>
 *
 * @since	C10
 */
public class IpcEventSystemNotReadyException extends IpcException
{
	final static long  serialVersionUID = -4498642047985444227L;

	/**
	 * Construct an {@code IpcEventSystemNotReadyException} without
	 * detailed message.
	 */
	public IpcEventSystemNotReadyException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcEventSystemNotReadyException} with the
	 * specified detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcEventSystemNotReadyException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcEventSystemNotReadyException} with the
	 * specified detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcEventSystemNotReadyException(String message,
						Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcEventSystemNotReadyException} with the
	 * specified cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcEventSystemNotReadyException(Throwable cause)
	{
		super(cause);
	}
}
