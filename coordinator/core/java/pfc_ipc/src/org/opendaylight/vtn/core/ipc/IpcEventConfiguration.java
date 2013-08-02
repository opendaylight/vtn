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
 *   The {@code IpcEventConfiguration} class represents a set of parameters
 *   which is used to initialize IPC event system by IPC client.
 * </p>
 * <p>
 *   Note that this class is not synchronized.
 *   Concurrent access to an {@code IpcEventConfiguration} from multiple
 *   threads results in undefined behavior.
 * </p>
 *
 * @since	C10
 * @see		IpcEventSystem#initialize(IpcEventConfiguration)
 */
public final class IpcEventConfiguration
{
	/**
	 * An idle timeout, in milliseconds, of event listener task thread.
	 */
	private int  _idleTimeout;

	/**
	 * Maximum number of event listener task threads.
	 */
	private int  _maxThreads;

	/**
	 * Reconnection interval, in milliseconds.
	 */
	private int  _connectInterval;

	/**
	 * I/O timeout in milliseconds.
	 */
	private int  _timeout;

	/**
	 * Construct a new IPC event configuration object.
	 * All parameters are initialized with the default value.
	 */
	public IpcEventConfiguration()
	{
	}

	/**
	 * <p>
	 *   Specify how long an event task thread should wait for a new task.
	 * </p>
	 * <p>
	 *   The IPC event subsystem in the IPC client library executes task
	 *   on a temporary native thread called event task thread.
	 *   When an event task thread becomes idle, it will wait for a new
	 *   task at most the specified milliseconds, and it will quit if no
	 *   task is posted.
	 * </p>
	 * <p>
	 *   {@code msec} value must be {@code 10 <= msec <= 86400000} or zero.
	 *   Zero means the system default value, which is 1000.
	 *   <strong>Note that this method never verifies the specified
	 *   value.</strong>
	 *   {@link IpcEventSystem#initialize(IpcEventConfiguration)}
	 *   throws an {@code IllegalArgumentException} if it detects
	 *   at least one invalid parameter in the given
	 *   {@code IpcEventConfiguration} instance.
	 * </p>
	 *
	 * @param msec		How long, in milliseconds, an event task thread
	 *			should wait for a new task.
	 */
	public void setIdleTimeout(int msec)
	{
		_idleTimeout = msec;
	}

	/**
	 * Return how long, in milliseconds, an event task thread should wait
	 * for a new task.
	 *
	 * @return	How long, in milliseconds, an event task thread should
	 *		wait for a new task. Zero means the system default
	 *		value.
	 * @see #setIdleTimeout(int)
	 */
	public int getIdleTimeout()
	{
		return _idleTimeout;
	}

	/**
	 * <p>
	 *   Specify the upper limit of the number of event task threads.
	 * </p>
	 * <p>
	 *   {@code nthreads} must be {@code 0 <= nthreads <= 1024}.
	 *   Zero means the system default value, which is 32.
	 *   <strong>Note that this method never verifies the specified
	 *   value.</strong>
	 *   {@link IpcEventSystem#initialize(IpcEventConfiguration)}
	 *   throws an {@code IllegalArgumentException} if it detects
	 *   at least one invalid parameter in the given
	 *   {@code IpcEventConfiguration} instance.
	 *
	 * @param nthreads	The maximum number of event task threads.
	 */
	public void setMaxThreads(int nthreads)
	{
		_maxThreads = nthreads;
	}

	/**
	 * Return the maximum number of event task threads.
	 *
	 * @return	The maximum number of event task threads.
	 *		Zero means the system default value.
	 * @see	#setMaxThreads(int)
	 */
	public int getMaxThreads()
	{
		return _maxThreads;
	}

	/**
	 * <p>
	 *   Specify interval, in milliseconds, between reconnection to
	 *   IPC server.
	 * </p>
	 * <p>
	 *   The IPC client library tries to establish the connection to
	 *   IPC server asynchronously when an IPC event handler is registered.
	 *   If it fails to connect, the IPC client library will try again
	 *   periodically until the connection is established.
	 * </p>
	 * <p>
	 *   {@code msec} must be {@code 1000 <= msec <= 86400000} or zero.
	 *   Zero means the system default value, which is 60000.
	 *   <strong>Note that this method never verifies the specified
	 *   value.</strong>
	 *   {@link IpcEventSystem#initialize(IpcEventConfiguration)}
	 *   throws an {@code IllegalArgumentException} if it detects
	 *   at least one invalid parameter in the given
	 *   {@code IpcEventConfiguration} instance.
	 * </p>
	 *
	 * @param msec		Interval, in milliseconds, between attempt to
	 *			reconnect to IPC server.
	 */
	public void setConnectInterval(int msec)
	{
		_connectInterval = msec;
	}

	/**
	 * Return the number of milliseconds between attempt to reconnect to
	 * IPC server.
	 *
	 * @return	The number of milliseconds between attempt to reconnect
	 *		to IPC server. Zero means the system default value.
	 * @see #setConnectInterval(int)
	 */
	public int getConnectInterval()
	{
		return _connectInterval;
	}

	/**
	 * <p>
	 *   Specify I/O timeout, in milliseconds, of a transaction on a
	 *   IPC event listener session.
	 * </p>
	 * <p>
	 *   The IPC client library discards the event listener session
	 *   if one transaction on the session does not complete within
	 *   the specified milliseconds.
	 * </p>
	 * <p>
	 *   {@code msec} must be {@code 1000 <= msec <= 3600000} or zero.
	 *   Zero means the system default value, which is 10000.
	 *   <strong>Note that this method never verifies the specified
	 *   value.</strong>
	 *   {@link IpcEventSystem#initialize(IpcEventConfiguration)}
	 *   throws an {@code IllegalArgumentException} if it detects
	 *   at least one invalid parameter in the given
	 *   {@code IpcEventConfiguration} instance.
	 * </p>
	 *
	 * @param msec		How long, in milliseconds, the IPC client
	 *			library will wait for the completion of
	 *			one I/O transaction.
	 */
	public void setTimeout(int msec)
	{
		_timeout = msec;
	}

	/**
	 * Return how long, in milliseconds, the IPC client library will wait
	 * for the completion of one I/O transaction on an event listener
	 * session.
	 *
	 * @return	How long, in milliseconds, the IPC client library
	 *		will wait for the completion of one I/O transaction
	 *		on an event listener session. Zero means the system
	 *		default value.
	 */
	public int getTimeout()
	{
		return _timeout;
	}
}
