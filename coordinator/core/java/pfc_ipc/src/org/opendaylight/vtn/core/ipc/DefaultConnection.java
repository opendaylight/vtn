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
 *   The {@code DefaultConnection} class is a single global IPC connection
 *   shared with all components in a process.
 * </p>
 * <p>
 *   You can obtain {@code DefaultConnection} instance by
 *   {@link #getInstance()}.
 * </p>
 * <blockquote><pre>
 * DefaultConnection  conn = DefaultConnection.getInstance();</pre>
 * </blockquote>
 * <p>
 *   By default, the IPC channel address of the default connection is
 *   configured as "pfcd" on the local host. It can be changed by
 *   {@link #setAddress(ChannelAddress)}.
 * </p>
 *
 * @since	C10
 */
public final class DefaultConnection extends IpcConnection
{
	/**
	 * Load native library.
	 */
	static {
		ClientLibrary.load();
	}

	/**
	 * IPC connection handle associated with the default connection.
	 */
	private final static int  CONN_DEFAULT = 1;

	/**
	 * Holder class of the default connection.
	 */
	private final static class DefaultHolder
	{
		/**
		 * The default connection instance.
		 */
		private final static DefaultConnection _theInstance =
			new DefaultConnection();
	}

	/**
	 * Return the default connection instance.
	 *
	 * @return	The default connection instance.
	 */
	public static DefaultConnection getInstance()
	{
		return DefaultHolder._theInstance;
	}

	/**
	 * Construct the default connection instance.
	 */
	private DefaultConnection()
	{
		super(CONN_DEFAULT);
	}

	/**
	 * This method does nothing because the default connection can not
	 * be closed.
	 */
	@Override
	public void close()
	{
	}

	/**
	 * Create a new IPC client session handle on the default connection.
	 *
	 * @param handle	Ignored. This method always uses the default
	 *			connection.
	 * @param name		IPC service name.
	 * @param service	IPC service ID.
	 * @param flags		Session creation flag bits.
	 * @return		An IPC client session handle.
	 * @throws IllegalArgumentException
	 *	Invalid creation flag is set to {@code flags}.
	 * @throws IllegalArgumentException
	 *	Invalid IPC service name is specified to {@code name}.
	 * @throws IpcShutdownException
	 *	The connection is closed while processing.
	 */
	@Override
	native long createSession(int handle, String name, int service,
				  int flags) throws IpcException;

	/**
	 * <p>
	 *   Change the IPC channel address of the default connection.
	 * </p>
	 * <p>
	 *   Note that this method does not affect existing alternative
	 *   connections.
	 * </p>
	 *
	 * @param chaddr	A new IPC channel address.
	 * @throws NullPointerException
	 *	{@code chaddr} is null.
	 * @throws IllegalArgumentException
	 *	{@code chaddr} is an invalid IPC channel address.
	 * @throws IpcResourceBusyException
	 *	At least one client session exists on the default connection.
	 */
	public void setAddress(ChannelAddress chaddr) throws IpcException
	{
		setAddress(chaddr.toString());
	}

	/**
	 * Change the IPC channel address of the default connection.
	 *
	 * @param chaddr	A new IPC channel address.
	 */
	private native void setAddress(String chaddr) throws IpcException;
}
