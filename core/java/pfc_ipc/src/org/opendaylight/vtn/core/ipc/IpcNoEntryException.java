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
 *   The {@code IpcNoEntryException} class is an exception to notify that
 *   the specified target resource does not exist.
 *   Typically <strong>ENOENT</strong> error returned by the IPC framework
 *   library is reported by this exception.
 * </p>
 *
 * @since	C10
 */
public class IpcNoEntryException extends IpcException
{
	final static long  serialVersionUID = -2936229415281084117L;

	/**
	 * Construct an {@code IpcNoEntryException} without detailed
	 * message.
	 */
	public IpcNoEntryException()
	{
		super();
	}

	/**
	 * Construct an {@code IpcNoEntryException} with the specified
	 * detailed message.
	 *
	 * @param message	The detailed message.
	 */
	public IpcNoEntryException(String message)
	{
		super(message);
	}

	/**
	 * Construct an {@code IpcNoEntryException} with the specified
	 * detailed message and cause.
	 *
	 * @param message	The detailed message.
	 * @param cause		The cause of this exception.
	 */
	public IpcNoEntryException(String message, Throwable cause)
	{
		super(message, cause);
	}

	/**
	 * Construct an {@code IpcNoEntryException} with the specified
	 * cause.
	 *
	 * @param cause		The cause of this exception.
	 */
	public IpcNoEntryException(Throwable cause)
	{
		super(cause);
	}
}
