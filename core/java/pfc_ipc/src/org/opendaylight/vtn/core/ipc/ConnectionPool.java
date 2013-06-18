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
 *   The {@code ConnectionPool} class implements {@link AltConnection} cache.
 * </p>
 * <p>
 *   The global pool, which is a single global pool shared by all components
 *   in the Java VM, is provided by the system. The global pool can be
 *   obtained by {@link #getGlobalPool()}.
 * </p>
 * <p>
 *   An instance of {@code ConnectionPool} can cache one
 *   {@link AltConnection} per an IPC channel address.
 *   So {@link #open(ChannelAddress)} method call with specifying the same
 *   IPC channel address will return the same instance if the connection
 *   associated with the specified IPC channel address is cached in the pool.
 * </p>
 * <p>
 *   If an alternative connection associated with the connection pool
 *   is closed by {@link AltConnection#close()} or the GC,
 *   the connection pool tries to cache it in the pool without closing
 *   actual connection. {@link #discard(AltConnection)} can be used
 *   to close the cached connection by force.
 * </p>
 * <p>
 *   Note that the connection pool, except for the global pool, will be
 *   destroyed by the GC if all references to the pool are removed.
 *   All alternative connections in the pool will be closed when the pool
 *   is destroyed. So you must keep reference to {@code ConnectionPool}
 *   instance while it is used.
 *   Although unused connection pool will be destroyed by the GC, it is
 *   strongly recommended to destroy unused pool by {@link #destroy()}
 *   explicitly.
 * </p>
 *
 * @since	C10
 */
public final class ConnectionPool
{
	/**
	 * Load native library.
	 */
	static {
		ClientLibrary.load();
	}

	/**
	 * Invalid connection pool handle.
	 */
	private final static int  CPOOL_INVALID = 0;

	/**
	 * Connection pool handle for the global pool.
	 */
	private final static int  CPOOL_GLOBAL = 1;

	/**
	 * Flag bits for {@code pfc_ipcclnt_cpool_close()} which indicates
	 * the specified connection handle must be closed by force.
	 */
	private final static int  CPOOL_CLOSE_FORCE = 0x1;

	/**
	 * Connection pool handle of this instance.
	 */
	private int  _pool;

	/**
	 * Holder class of the global pool.
	 */
	private final static class GlobalPoolHolder
	{
		/**
		 * The global pool instance.
		 */
		private final static ConnectionPool _theInstance =
			new ConnectionPool(CPOOL_GLOBAL, false);
	}

	/**
	 * Get the global pool instance.
	 *
	 * @return	The global pool.
	 */
	public static ConnectionPool getGlobalPool()
	{
		return GlobalPoolHolder._theInstance;
	}

	/**
	 * <p>
	 *   Close the specified alternative connection by force.
	 * </p>
	 * <p>
	 *   This method closes the specified connection even if it is still
	 *   in use. Typically {@link AltConnection#close()} should be used
	 *   to close the connection.
	 * </p>
	 *
	 * @param conn		An alternative connection to be closed.
	 * @throws NullPointerException
	 *	{@code conn} is null.
	 * @throws IpcNoEntryException
	 *	The specified connection does not belong to any connection
	 *	pool.
	 */
	public static void discard(AltConnection conn) throws IpcException
	{
		if (!(conn instanceof PooledConnection)) {
			if (conn == null) {
				throw new NullPointerException("conn is null");
			}

			throw new IpcNoEntryException
				("Not pooled connection.");
		}

		// Invalidate connection handle.
		int handle = conn.invalidate();
		if (handle == IpcConnection.CONN_INVALID) {
			// Already closed.
			return;
		}

		try {
			PooledConnection pconn = (PooledConnection)conn;
			int pool = pconn.getPoolHandle();
			close(pool, handle, CPOOL_CLOSE_FORCE);
		}
		catch (IpcBadPoolException e) {
			// The connection pool is already destroyed.
			// So we can ignore this error.
		}
	}

	/**
	 * <p>
	 *   Create a new connection pool.
	 * </p>
	 * <p>
	 *   The capacity of the connection pool is determined by the system.
	 * </p>
	 *
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	public ConnectionPool() throws IpcException
	{
		this(0);
	}

	/**
	 * <p>
	 *   Create a new connection pool with specifying the capacity.
	 * </p>
	 * <p>
	 *   {@code capacity} specifies the maximum number of connections
	 *   in a new connection pool. It is determined by the system if
	 *   zero is specified.
	 * </p>
	 *
	 * @param capacity	The maximum number of connections in the pool.
	 * @throws IllegalArgumentException
	 *	A negative value is specified to {@code capacity}.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	public ConnectionPool(int capacity) throws IpcException
	{
		this(create(capacity), false);
	}

	/**
	 * Create the global pool instance.
	 *
	 * @param pool		A connection pool handle.
	 * @param dummy		A dummy parameter to distinguish this
	 *			constructor from others.
	 */
	private ConnectionPool(int pool, boolean dummy)
	{
		_pool = pool;
	}

	/**
	 * <p>
	 *   Destroy the connection pool, and closes all connections cached
	 *   in this pool.
	 * </p>
	 * <p>
	 *   Note that the global pool can not be destroyed.
	 *   If this method is called with the global pool, this method only
	 *   closes all cached connections.
	 * </p>
	 * <p>
	 *   This method does nothing if the pool is already destroyed.
	 * </p>
	 *
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	public void destroy() throws IpcException
	{
		int  pool;

		synchronized (this) {
			pool = _pool;
			if (pool == CPOOL_INVALID) {
				return;
			}
			if (pool != CPOOL_GLOBAL) {
				_pool = CPOOL_INVALID;
			}
		}

		try {
			destroy(pool);
		}
		catch (IpcBadPoolException e) {
			// Already destroyed.
		}
	}

	/**
	 * <p>
	 *   Create a new alternative connection via this connection pool.
	 * </p>
	 * <p>
	 *   If a connection associated with the specified IPC channel address
	 *   is cached in the pool, this method returns it. Otherwise it
	 *   creates a new connection and caches it in the pool if possible.
	 * </p>
	 *
	 * @param chaddr	An IPC channel address.
	 *			If null is specified, current IPC channel
	 *			address of {@link DefaultConnection} is used.
	 * @throws IllegalArgumentException
	 *	An invalid IPC channel address is specified to {@code chaddr}.
	 * @throws IpcBadPoolException
	 *	This connection pool is already destroyed.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	public AltConnection open(ChannelAddress chaddr) throws IpcException
	{
		int pool;

		synchronized (this) {
			pool = _pool;
			if (pool == CPOOL_INVALID) {
				throw new IpcBadPoolException
					("Already destroyed.");
			}
		}

		// Open an alternative connection handle via this pool.
		String text = (chaddr == null) ? null : chaddr.toString();

		return new PooledConnection(pool, open(pool, text));
	}

	/**
	 * Return the number of connections cached in this pool.
	 *
	 * @return	The number of connections in this pool.
	 * @throws IpcBadPoolException
	 *	This connection pool is already destroyed.
	 */
	public int getSize() throws IpcException
	{
		return getSize(getPoolHandle());
	}

	/**
	 * Return the capacity of this pool, which is the maximum number of
	 * connections.
	 *
	 * @return	The capacity of this pool.
	 * @throws IpcBadPoolException
	 *	This connection pool is already destroyed.
	 */
	public int getCapacity() throws IpcException
	{
		return getCapacity(getPoolHandle());
	}

	/**
	 * Ensure that a connection pool handle is destroyed when there
	 * are no more references to this instance.
	 *
	 * @throws Throwable
	 *	An error occurs while finalization.
	 */
	@Override
	protected void finalize() throws Throwable
	{
		destroy();
	}

	/**
	 * Create a new connection pool handle.
	 *
	 * @param capacity	The maximum number of connections in the pool.
	 * @return		A connection pool handle.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	private static native int create(int capacity) throws IpcException;

	/**
	 * Destroy the connection pool associated with the specified
	 * connection pool handle.
	 *
	 * @param handle	A connection pool handle.
	 * @throws IpcBadPoolException
	 *	This connection pool is already destroyed.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	private static native void destroy(int handle) throws IpcException;

	/**
	 * Create an alternative connection via the specified connection pool
	 * handle.
	 *
	 * @param pool		Connection pool handle.
	 * @param chaddr	IPC channel address.
	 * @return		Connection handle associated with a created
	 *			connection.
	 * @throws IllegalArgumentException
	 *	An invalid IPC channel address is specified to
	 *	{@code chaddr}.
	 * @throws IpcBadPoolException
	 *	This connection pool is already destroyed.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	private static native int open(int pool, String chaddr)
		throws IpcException;

	/**
	 * Close the alternative connection in the specified connection pool.
	 *
	 * @param pool		Connection pool handle.
	 * @param handle	Alternative connection handle.
	 * @param flags		Flags to be passed to
	 *			{@code pfc_ipcclnt_cpool_close()}.
	 * @throws IpcBadPoolException
	 *	This connection pool is already destroyed.
	 * @throws IpcNoEntryException
	 *	The specified connection does not belong to any connection
	 *	pool.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	private static native void close(int pool, int handle, int flags)
		throws IpcException;

	/**
	 * <p>
	 *   Scan all connection pools, and reap unused connections.
	 * </p>
	 * <p>
	 *   If {@code forced} is true, this method closes all unused
	 *   connections in connection pools. It is expected to be called
	 *   when the system is very low on memory.
	 * </p>
	 * <p>
	 *   If {@code forced} is false, this method closes only recently unused
	 *   connections in connection pools. This method call with specifying
	 *   false to {@code force} must be done periodically.
	 * </p>
	 *
	 * @param forced	A boolean value which determines behavior
	 *			of this method.
	 */
	public static native void reap(boolean forced);

	/**
	 * Return the connection pool handle.
	 *
	 * @return	The connection pool handle associated with this
	 *		instance.
	 */
	private synchronized int getPoolHandle()
	{
		return _pool;
	}

	/**
	 * Return the number of connections cached in this pool.
	 *
	 * @param pool		Connection pool handle.
	 * @return		The number of connections in this pool.
	 * @throws IpcBadPoolException
	 *	This connection pool is already destroyed.
	 */
	private native int getSize(int pool);

	/**
	 * Return the capacity of this pool, which is the maximum number of
	 * connections.
	 *
	 * @param pool		Connection pool handle.
	 * @return		The capacity of this pool.
	 * @throws IpcBadPoolException
	 *	This connection pool is already destroyed.
	 */
	private native int getCapacity(int pool);

	/**
	 * Implementation of alternative connection created from
	 * connection pool.
	 */
	private final static class PooledConnection extends AltConnection
	{
		private final int _pool;

		/**
		 * Create an alternative connection which connects to the
		 * specified IPC channel address.
		 *
		 * @param pool		Connection pool handle.
		 * @param handle	Alternative connection handle.
		 */
		private PooledConnection(int pool, int handle)
		{
			super(handle);
			_pool = pool;
		}

		/**
		 * Return connection pool handle associated with this
		 * connection.
		 *
		 * @return	A connection pool associated with this
		 *		connection.
		 */
		private int getPoolHandle()
		{
			return _pool;
		}

		/**
		 * This method must not be called.
		 *
		 * @throws IllegalStateException
		 *	Always thrown.
		 */
		@Override
		int openImpl(String chaddr)
		{
			throw new IllegalStateException("Unexpected call.");
		}

		/**
		 * Close an alternative connection associated with the
		 * specified connection handle.
		 *
		 * @param handle	A connection handle to be closed.
		 * @throws IpcLibraryException
		 *	The IPC client library returned an error.
		 */
		@Override
		void closeImpl(int handle) throws IpcException
		{
			int pool = getPoolHandle();
			try {
				ConnectionPool.close(pool, handle, 0);
			}
			catch (IpcBadPoolException e) {
				// This pool is already destroyed.
				// So we can ignore this error.
			}
		}
	}
}
