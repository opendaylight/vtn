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
 *   An instance of {@code AltConnection} class represents an alternative
 *   connection, which can be created dynamically.
 * </p>
 * <p>
 *   If more than one IPC service requests are issued on a same connection,
 *   they are serialized and processed one by one. So if you want to issue
 *   more than one IPC service requests in parallel, you need to create
 *   more than one alternative connections.
 * </p>
 * <p>
 *   Use {@link #open(ChannelAddress)} if you want to create a new
 *   alternative connection which connects "foo" service on the local host.
 * </p>
 * <blockquote><pre>
 *ChannelAddress chaddr = new ChannelAddress("foo");
 *AltConnection  conn = AltConnection.open(chaddr);</pre>
 * </blockquote>
 * <p>
 *   Note that the connection will be closed by the GC if all references to
 *   the connection are removed. You must keep reference to
 *   {@code AltConnection} instance while it is used.
 *   Although unused connection will be closed by the GC, it is strongly
 *   recommended to close unused connection by {@link #close()} explicitly.
 * </p>
 * <p>
 *   The {@link ConnectionPool} class can be used to cache alternative
 *   connections.
 * </p>
 *
 * @since	C10
 * @see		ChannelAddress
 * @see		ConnectionPool
 */
public class AltConnection extends IpcConnection
{
	/**
	 * Load native library.
	 */
	static {
		ClientLibrary.load();
	}

	/**
	 * <p>
	 *   Create a new alternative connection which connects to the IPC
	 *   service specified by {@code chaddr}.
	 * </p>
	 *
	 * @param chaddr	An IPC channel address.
	 *			If null is specified, current IPC channel
	 *			address of {@link DefaultConnection} is used.
	 * @return		An {@code AltConnection} instance associated
	 *			with an alternative connection.
	 * @throws IllegalArgumentException
	 *	IPC channel name in {@code chaddr} is invalid.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	public static AltConnection open(ChannelAddress chaddr)
		throws IpcException
	{
		return new AltConnection(chaddr);
	}

	/**
	 * Create an alternative connection which connects to the specified
	 * IPC channel address.
	 *
	 * @param chaddr	An IPC channel address.
	 *			If null is specified, current IPC channel
	 *			address of {@link DefaultConnection} is used.
	 * @throws IllegalArgumentException
	 *	IPC channel name in {@code chaddr} is invalid.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	private AltConnection(ChannelAddress chaddr) throws IpcException
	{
		super();

		String text = (chaddr == null) ? null : chaddr.toString();
		_handle = openImpl(text);
	}

	/**
	 * Create an alternative connection instance associated with the
	 * specified connection handle.
	 *
	 * @param conn		IPC connection handle.
	 */
	AltConnection(int conn)
	{
		super(conn);
	}

	/**
	 * <p>
	 *   Close IPC connection.
	 * </p>
	 * <p>
	 *   After the call of this method, no further IPC service request
	 *   on this connection is refused.
	 *   This method does nothing if this instance is already closed.
	 * </p>
	 *
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	@Override
	public void close() throws IpcException
	{
		int handle;

		synchronized (this) {
			handle = _handle;
			if (handle == CONN_INVALID) {
				return;
			}
			_handle = CONN_INVALID;
		}

		closeImpl(handle);
	}

	/**
	 * Create a new IPC client session handle on this alternative
	 * connection.
	 *
	 * @param handle	An IPC connection handle.
	 * @param name		IPC service name.
	 * @param service	IPC service ID.
	 * @param flags		Session creation flag bits.
	 * @return		An IPC client session handle.
	 * @throws IllegalArgumentException
	 *	Invalid creation flag is set to {@code flags}.
	 * @throws IllegalArgumentException
	 *	Invalid IPC service name is specified to {@code name}.
	 * @throws IpcBadConnectionException
	 *	Invalid connection handle is specified to {@code handle}.
	 * @throws IpcShutdownException
	 *	The connection is closed while processing.
	 */
	@Override
	native long createSession(int handle, String name, int service,
				  int flags) throws IpcException;

	/**
	 * Ensure that an alternative connection handle is closed when there
	 * are no more references to this instance.
	 *
	 * @throws Throwable
	 *	An error occurs while finalization.
	 */
	@Override
	protected void finalize() throws Throwable
	{
		close();
	}

	/**
	 * Invalidate this connection without closing actual connection handle.
	 *
	 * @return	Old IPC connection handle.
	 */
	synchronized int invalidate()
	{
		int handle = _handle;

		_handle = CONN_INVALID;

		return handle;
	}

	/**
	 * Create an alternative connection which connects to the specified
	 * IPC channel address.
	 *
	 * @param chaddr	IPC channel address.
	 * @return		Connection handle associated with a created
	 *			connection.
	 * @throws IllegalArgumentException
	 *	An invalid IPC channel address is specified to {@code chaddr}.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	native int openImpl(String chaddr) throws IpcException;

	/**
	 * Close an alternative connection associated with the specified
	 * connection handle.
	 *
	 * @param handle	A connection handle to be closed.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	native void closeImpl(int handle) throws IpcException;
}
