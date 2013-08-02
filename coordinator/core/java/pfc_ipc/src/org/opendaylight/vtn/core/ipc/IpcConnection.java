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
 *   The {@code IpcConnection} class is the base class of the IPC connection.
 *   The IPC connection is a pseudo connection between IPC client and IPC
 *   server. An instance of {@code IpcConnection} is used by IPC client to
 *   issue IPC service request to IPC server.
 * </p>
 * <p>
 *   IPC connection has two types:
 * </p>
 * <dl>
 *   <dt>Default Connection</dt>
 *   <dl>
 *     <p>
 *       A single global connection shared with all components in a Java
 *       VM. See {@link DefaultConnection} for more details.
 *     </p>
 *   </dl>
 *   <dt>Alternative Connection</dt>
 *   <dl>
 *     <p>
 *       IPC connection which can be created dynamically.
 *       See {@link AltConnection} for more details.
 *     </p>
 *   </dl>
 * <dl>
 *
 * @since	C10
 * @see		DefaultConnection
 * @see		AltConnection
 * @see		ConnectionPool
 */
public abstract class IpcConnection
{
	/**
	 * Invalid IPC connection handle.
	 */
	final static int  CONN_INVALID = 0;

	/**
	 * IPC connection handle.
	 */
	int  _handle;

	/**
	 * Create an invalid IPC connection.
	 */
	IpcConnection()
	{
		_handle = CONN_INVALID;
	}

	/**
	 * Create an IPC connection instance associated with the specified
	 * connection handle.
	 *
	 * @param conn		IPC connection handle.
	 */
	IpcConnection(int conn)
	{
		_handle = conn;
	}

	/**
	 * <p>
	 *   Close IPC connection.
	 * </p>
	 * <p>
	 *   After the call of this method, any further IPC service request
	 *   on this connection is refused.
	 *   This method does nothing if this instance is already closed.
	 * </p>
	 *
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	public abstract void close() throws IpcException;

	/**
	 * <p>
	 *   Create a new IPC client session on this connection, with
	 *   specifying default creation flag.
	 * </p>
	 * <p>
	 *   This method is identical to
	 *   {@link #createSession(String, int, int)} with specifying zero
	 *   to {@code flags}.
	 * </p>
	 *
	 * @param name		IPC service name.
	 * @param service	IPC service ID.
	 * @return		An IPC client session.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	Invalid IPC service name is specified to {@code name}.
	 * @throws IpcBadConnectionException
	 *	This connection is already closed.
	 * @throws IpcShutdownException
	 *	This connection is closed while processing.
	 */
	public final ClientSession createSession(String name, int service)
		throws IpcException
	{
		return createSession(name, service, 0);
	}

	/**
	 * <p>
	 *   Create a new IPC client session on this connection, with
	 *   specifying creation flag.
	 * </p>
	 * <p>
	 *   {@code flags} is either zero or the bitwise OR of one or more
	 *   of the following flags.
	 * </p>
	 * <dl>
	 *   <dt>{@link ClientSession#C_CANCELABLE}</dt>
	 *   <dd>
	 *     <p>
	 *       Enable session-specific cancellation.
	 *     </p>
	 *     <p>
	 *       An IPC session created with this flag can be canceled by the
	 *       call of {@link ClientSession#cancel()} or
	 *       {@link ClientSession#discard()} without any side effect to
	 *       other IPC sessions.
	 *     </p>
	 *   </dd>
	 *   <dt>{@link ClientSession#C_NOGLOBCANCEL}</dt>
	 *   <dd>
	 *     <p>
	 *       Ignore global cancellation by {@link ClientLibrary#cancel()}.
	 *       This flag implies {@link ClientSession#C_CANCELABLE}.
	 *     </p>
	 *     <p>
	 *       An IPC service request on a session created with this flag
	 *       will never be canceled by {@link ClientLibrary#cancel()}.
	 *     </p>
	 *   </dd>
	 * </dl>
	 *
	 * @param name		IPC service name.
	 * @param service	IPC service ID.
	 * @param flags		Session creation flag bits.
	 * @return		An IPC client session.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	Invalid creation flag is set to {@code flags}.
	 * @throws IllegalArgumentException
	 *	Invalid IPC service name is specified to {@code name}.
	 * @throws IpcBadConnectionException
	 *	This connection is already closed.
	 * @throws IpcShutdownException
	 *	This connection is closed while processing.
	 */
	public final ClientSession createSession(String name, int service,
						 int flags)
		throws IpcException
	{
		if (name == null) {
			throw new NullPointerException
				("IPC service name is null.");
		}

		int handle;
		synchronized (this) {
			handle = _handle;
			if (handle == CONN_INVALID) {
				throw new IpcBadConnectionException
					("Already closed.");
			}
		}

		long session = createSession(handle, name, service, flags);
		boolean succeeded = false;
		try {
			ClientSession  sess = new ClientSession(session);
			succeeded = true;

			return sess;
		}
		finally {
			if (!succeeded) {
				ClientSession.destroy(session);
			}
		}
	}

	/**
	 * <p>
	 *   Create a new IPC client session on this connection.
	 * </p>
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
	abstract long createSession(int handle, String name, int service,
				    int flags) throws IpcException;

	/**
	 * Get IPC connection handle of this instance.
	 *
	 * @return	IPC connection handle.
	 */
	synchronized int getHandle()
	{
		return _handle;
	}
}
