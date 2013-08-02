/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import org.opendaylight.vtn.core.util.TimeSpec;

/**
 * <p>
 *   An instance of the {@code ClientSession} class represents an IPC client
 *   session, which is a context to issue IPC service request.
 *   IPC client session can be created by the call of
 *   {@link IpcConnection#createSession(String, int)} or
 *   {@link IpcConnection#createSession(String, int, int)}.
 * </p>
 * <p>
 *   An IPC client session is destroyed when a {@code ClientSession}
 *   is finalized by the GC. But it is strongly recommended to destroy
 *   a client session explicitly by {@link #destroy()}, because it may
 *   keep huge amount of additional data.
 * </p>
 * <p>
 *   IPC client session has the following internal state:
 * </p>
 * <dl>
 *   <dt>READY</dt>
 *   <dd>
 *     <p>
 *       An IPC client session can issue an IPC service request.
 *       Additional data to be sent to IPC server can be appended to the
 *       session.
 *     </p>
 *     <p>
 *       This is the initial state.
 *     </p>
 *   </dd>
 *   <dt>BUSY</dt>
 *   <dd>
 *     <p>
 *       Issuing an IPC service request. Internal state will be changed to
 *       <strong>RESULT</strong> when IPC client finishes receiving response
 *       from IPC server.
 *     </p>
 *   </dd>
 *   <dt>RESULT</dt>
 *   <dd>
 *     <p>
 *       IPC client session keeps response from IPC server.
 *       Additional data sent by IPC server can be derived from IPC client
 *       session. Internal state will be changed to <strong>READY</strong>
 *       by the call of {@link #reset(String, int)}.
 *     </p>
 *   </dd>
 *   <dt>DISCARD</dt>
 *   <dd>
 *     <p>
 *       IPC client session is no longer usable, and it should be destroyed
 *       immediately.
 *     </p>
 *   </dd>
 * </dl>
 * <p>
 *   Although the <code>ClientSession</code> class is thread safe, it is
 *   strongly discouraged to share one <code>ClienSession</code> with
 *   multiple threads.
 * </p>
 *
 * @since	C10
 */
public class ClientSession
{
	/**
	 * <p>
	 *   Load native library.
	 * </p>
	 */
	static {
		ClientLibrary.load();
	}

	/**
	 * <p>
	 *   Session creation flag bit which indicates a new session must enable
	 *   session-specific cancellation.
	 * </p>
	 */
	public final static int  C_CANCELABLE = 0x1;

	/**
	 * <p>
	 *   Session creation flag bit which indicates a new session must ignore
	 *   global cancellation. This flag implies {@link #C_CANCELABLE}.
	 * </p>
	 */
	public final static int  C_NOGLOBCANCEL = 0x2;

	/**
	 * <p>
	 *   IPC service response code which indicates a fatal error.
	 * </p>
	 */
	public final static int  RESP_FATAL = -1;

	/**
	 * <p>
	 *   Invalid client session handle.
	 * </p>
	 */
	final static long  SESS_INVALID = 0L;

	/**
	 * <p>
	 *   IPC client session handle.
	 * </p>
	 */
	long  _session;

	/**
	 * <p>
	 *   Busy counter which protects IPC client session handle.
	 * </p>
	 */
	private long  _busy = 1;

	/**
	 * <p>
	 *   Construct a new IPC client session.
	 * </p>
	 *
	 * @param session	An IPC client session handle.
	 */
	ClientSession(long session)
	{
		_session = session;
	}

	/**
	 * <p>
	 *   Reset state of this session to <strong>READY</strong>.
	 * </p>
	 * <p>
	 *   If you want to issue another IPC service request on the IPC
	 *   client session after the call of {@link #invoke()}, you must
	 *   reset the session with this method.
	 *   Note that all additional data received from the IPC server will
	 *   be discarded by this method.
	 * </p>
	 *
	 * @param name		IPC service name.
	 * @param service	IPC service ID.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	Invalid IPC service name is specified to {@code name}.
	 * @throws IpcBadClientSessionException
	 *	This session is already destroyed.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on this session.
	 * @throws IpcShutdownException
	 *	The state of this session is <strong>DISCARD</strong>.
	 *	This exception indicates that this session is no longer
	 *	available.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	public final void reset(String name, int service) throws IpcException
	{
		long session = getSession();

		try {
			reset(session, name, service);
		}
		finally {
			release(session);
		}
	}

	/**
	 * <p>
	 *   Invoke an IPC service on the IPC server.
	 * </p>
	 * <p>
	 *   This method sends data listed below:
	 * </p>
	 * <ul>
	 *   <li>
	 *     <p>
	 *       A pair of IPC service name and ID specified by
	 *       {@link IpcConnection#createSession(String, int)} or
	 *       {@link #reset(String, int)}.
	 *     </p>
	 *   </li>
	 *   <li>
	 *     <p>
	 *       All additional data added by {@link #addOutput(IpcDataUnit)}.
	 *       Note that they are always removed from this session
	 *       unless another thread is invoking an IPC service on the
	 *       same session.
	 *     </p>
	 *   </li>
	 * </ul>
	 * <p>
	 *   The caller must ensure that the state of this session is
	 *   <strong>READY</strong>.
	 *   After successful return of this method, the state of this session
	 *   is changed to <strong>RESULT</strong>.
	 * </p>
	 *
	 * @return	Response code received from the IPC server.
	 *		{@link #RESP_FATAL} means a fatal error.
	 * @throws IpcBadClientSessionException
	 *	This session is already destroyed.
	 * @throws IpcTimedOutException
	 *	An IPC service did not complete within the IPC client session
	 *	timeout.
	 * @throws IpcConnectionRefusedException
	 *	The IPC server is not running.
	 * @throws IpcConnectionResetException
	 *	The IPC server reset the connection while the client received
	 *	data.
	 * @throws IpcBrokenPipeException
	 *	The IPC server reset the connection while the client sent data.
	 * @throws IpcBadProtocolException
	 *	The IPC server sent unexpected data.
	 * @throws IpcPermissionDeniedException
	 *	The IPC server refused the connection because of authentication
	 *	failure.
	 * @throws IpcTooManyClientsException
	 *	The IPC server refused the connection because of too many
	 *	connections.
	 * @throws IpcUnknownServiceException
	 *	The IPC server did not recognize IPC service specified by
	 *	the IPC service name and IPC service ID.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being invoked on another thread.
	 * @throws IpcShutdownException
	 *	The state of this session is <strong>DISCARD</strong>.
	 *	This exception indicates that this session is no longer
	 *	available.
	 * @throws IpcBadStateException
	 *	The state of this session is <strong>RESULT</strong>.
	 *	This session must be reset by {@link #reset(String, int)}
	 *	before invoking IPC service.
	 * @throws IpcCanceledException
	 *	An IPC service was canceled by {@link ClientLibrary#cancel()}
	 *	or {@link #cancel()} or {@link #discard()}.
	 * @throws IpcClientDisabledException
	 *	The IPC client library is disabled permanently by
	 *	{@link ClientLibrary#disable()}.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	public final int invoke() throws IpcException
	{
		long session = getSession();

		try {
			return invoke(session);
		}
		finally {
			release(session);
		}
	}

	/**
	 * <p>
	 *   Add the specified data to the end of the additional data array.
	 * </p>
	 * <p>
	 *   This method must be called when the internal state of this session
	 *   is <strong>READY</strong>. All data added to the additional data
	 *   array will be sent to the IPC server by {@link #invoke()}.
	 * </p>
	 *
	 * @param unit	A data to be sent to the IPC server.
	 * @throws NullPointerException
	 *	{@code unit} is null.
	 * @throws IpcBadClientSessionException
	 *	This session is already destroyed.
	 * @throws IpcPermissionDeniedException
	 *	The additional data array is broken. If this exception is
	 *	thrown, this session must be destroyed by {@link #destroy()},
	 *	or reset by {@link #reset(String, int)}.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being invoked on another thread.
	 * @throws IpcShutdownException
	 *	The state of this session is <strong>DISCARD</strong>.
	 *	This exception indicates that this session is no longer
	 *	available.
	 * @throws IpcBadStateException
	 *	The state of this session is <strong>RESULT</strong>.
	 *	This session must be reset by {@link #reset(String, int)}
	 *	before adding data.
	 * @throws IpcTooBigDataException
	 *	No more data can not be added to the additional data array.
	 */
	public void addOutput(IpcDataUnit unit) throws IpcException
	{
		long session = getSession();

		try {
			unit.addClientOutput(session);
		}
		finally {
			release(session);
		}
	}

	/**
	 * <p>
	 *   Get an additional data received from the IPC server at the
	 *   specified additional array index.
	 * </p>
	 * <p>
	 *   {@code index} specifies the additional data array index.
	 *   The index of the first element is zero. It must be less than
	 *   the number of array elements obtained by
	 *   {@link #getResponseCount()}.
	 * </p>
	 * <p>
	 *   This method must be called when the internal state of this session
	 *   is <strong>RESULT</strong>.
	 * </p>
	 *
	 * @param index		Index of the additional data array.
	 * @return		An additional data received from the IPC
	 *			server.
	 * @throws IpcBadClientSessionException
	 *	This session is already destroyed.
	 * @throws IpcDataIndexOutOfBoundsException
	 *	{@code index} is greater than or equal to the number of
	 *	additional data array elements.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on this session.
	 * @throws IpcShutdownException
	 *	The state of this session is <strong>DISCARD</strong>.
	 *	This exception indicates that this session is no longer
	 *	available.
	 * @throws IpcBadStateException
	 *	The state of this session is <strong>READY</strong>.
	 * @throws IpcBadProtocolException
	 *	Received additional data is broken.
	 * @throws IpcUnknownStructException
	 *	This session keeps at least one unknown IPC structure in the
	 *	additional data array.
	 * @throws IpcStructLayoutMismatchException
	 *	Unexpected IPC structure layout was found in the additional
	 *	data array of this session.
	 * @see	#getResponseCount()
	 */
	public final IpcDataUnit getResponse(int index) throws IpcException
	{
		long session = getSession();

		try {
			return getResponse(session, index);
		}
		finally {
			release(session);
		}
	}

	/**
	 * <p>
	 *   Return the number of additional data array elements.
	 * </p>
	 *
	 * @return	The number of additional data array elements received
	 *		by the previous call of {@link #invoke()}.
	 * @throws IpcBadClientSessionException
	 *	This session is already destroyed.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on this session.
	 * @throws IpcShutdownException
	 *	The state of this session is <strong>DISCARD</strong>.
	 *	This exception indicates that this session is no longer
	 *	available.
	 * @throws IpcBadStateException
	 *	The state of this session is <strong>READY</strong>.
	 * @see	#getResponse(int)
	 */
	public final int getResponseCount() throws IpcException
	{
		long session = getSession();

		try {
			return getResponseCount(session);
		}
		finally {
			release(session);
		}
	}

	/**
	 * <p>
	 *   Copy all additional data in {@code sess} to this client session
	 *   for another IPC service request.
	 * </p>
	 * <p>
	 *   This method is identical to the following code.
	 * </p>
	 * <blockquote><pre>
	 * forward(sess, 0, sess.getResponseCount());</pre>
	 * </blockquote>
	 *
	 * @param sess		A client session which keeps additional data
	 *			received from the IPC server.
	 * @throws NullPointerException
	 *	{@code sess} is {@code null}.
	 * @throws IpcBadClientSessionException
	 *	This session or {@code sess} is already destroyed.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on this session or
	 *	{@code sess}.
	 * @throws IpcShutdownException
	 *	The state of this session or {@code sess} is
	 *	<strong>DISCARD</strong>.
	 * @throws IpcBadStateException
	 *	The state of this session is <strong>RESULT</strong>.
	 * @throws IpcBadStateException
	 *	The state of {@code sess} is <strong>READY</strong>.
	 * @throws IpcBadProtocolException
	 *	Additional data in {@code sess} is broken.
	 * @throws IpcUnknownStructException
	 *	{@code sess} keeps at least one unknown IPC structure in the
	 *	additional data array.
	 * @throws IpcStructLayoutMismatchException
	 *	Unexpected IPC structure layout was found in the additional
	 *	data array of {@code sess}.
	 * @throws IpcTooBigDataException
	 *	No more data can not be added to the additional data array.
	 * @see	#getResponseCount()
	 * @see	#forward(ClientSession, int, int)
	 */
	public final void forward(ClientSession sess) throws IpcException
	{
		forward(sess, 0, -1);
	}

	/**
	 * <p>
	 *   Copy additional data in {@code sess} to this client session
	 *   for another IPC service request.
	 * </p>
	 * <p>
	 *   This method copies all additional data at the index greater than
	 *   or equal to {@code beginIndex} in {@code sess}, as illustrated
	 *   the following code.
	 * </p>
	 * <blockquote><pre>
	 * forward(sess, beginIndex, sess.getResponseCount());</pre>
	 * </blockquote>
	 *
	 * @param sess		A client session which keeps additional data
	 *			received from the IPC server.
	 * @param beginIndex	The beginning (inclusive) index.
	 * @throws NullPointerException
	 *	{@code sess} is {@code null}.
	 * @throws IpcDataIndexOutOfBoundsException
	 *	{@code beginIndex} is greater than the length of the additional
	 *	data array in {@code sess}.
	 * @throws IpcBadClientSessionException
	 *	This session or {@code sess} is already destroyed.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on this session or
	 *	{@code sess}.
	 * @throws IpcShutdownException
	 *	The state of this session or {@code sess} is
	 *	<strong>DISCARD</strong>.
	 * @throws IpcBadStateException
	 *	The state of this session is <strong>RESULT</strong>.
	 * @throws IpcBadStateException
	 *	The state of {@code sess} is <strong>READY</strong>.
	 * @throws IpcBadProtocolException
	 *	Additional data in {@code sess} is broken.
	 * @throws IpcUnknownStructException
	 *	{@code sess} keeps at least one unknown IPC structure in the
	 *	additional data array.
	 * @throws IpcStructLayoutMismatchException
	 *	Unexpected IPC structure layout was found in the additional
	 *	data array of {@code sess}.
	 * @throws IpcTooBigDataException
	 *	No more data can not be added to the additional data array.
	 * @see	#getResponseCount()
	 * @see	#forward(ClientSession, int, int)
	 */
	public final void forward(ClientSession sess, int beginIndex)
		 throws IpcException
	{
		forward(sess, beginIndex, -1);
	}

	/**
	 * <p>
	 *   Copy additional data received from the IPC server to this
	 *   client session for another IPC service request.
	 * </p>
	 * <p>
	 *   {@code sess} must be either of the followings.
	 * </p>
	 * <ul>
	 *   <li>
	 *     A client session which keeps result of {@link #invoke()}.
	 *   </li>
	 *   <li>
	 *     A client session returned by {@link IpcEvent#getSession()}.
	 *   </li>
	 * </ul>
	 * <p>
	 *   This method copies additional data in the given client session
	 *   to the end of the additional data array in this session.
	 *   {@code beginIndex} and {@code endIndex} specifies the range of
	 *   additional data in {@code sess} to be copied. {@code beginIndex}
	 *   specifies the beginning index (inclusive), and {@code endIndex}
	 *   specifies the ending index (exclusive) of additional data.
	 *   If {@code endIndex} is a negative value, or is greater than
	 *   the length of the additional data array in {@code sess},
	 *   this method behaves as if the length of the additional data
	 *   array is specified to {@code endIndex}.
	 *   This method does nothing if the same value is specified to
	 *   {@code beginIndex} and {@code endIndex}.
	 * </p>
	 * <p>
	 *   This method works as illustrated in the following code.
	 * </p>
	 * <blockquote><pre>
	 * if (endIndex &lt; 0 || endIndex &gt; sess.getResponseCount()) {
	 *         endIndex = sess.getResponseCount();
	 * }
	 * for (int i = beginIndex; i &lt; endIndex; i++) {
	 *         addOutput(sess.getResponse(i));
	 * }</pre>
	 * </blockquote>
	 *
	 * @param sess		A client session which keeps additional data
	 *			received from the IPC server.
	 * @param beginIndex	The beginning (inclusive) index.
	 * @param endIndex	The ending (exclusive) index.
	 * @throws NullPointerException
	 *	{@code sess} is {@code null}.
	 * @throws IpcDataIndexOutOfBoundsException
	 *	{@code beginIndex} is greater than the length of the additional
	 *	data array in {@code sess}.
	 * @throws IpcDataIndexOutOfBoundsException
	 *	{@code beginIndex} is greater than {@code endIndex}.
	 * @throws IpcBadClientSessionException
	 *	This session or {@code sess} is already destroyed.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on this session or
	 *	{@code sess}.
	 * @throws IpcShutdownException
	 *	The state of this session or {@code sess} is
	 *	<strong>DISCARD</strong>.
	 * @throws IpcBadStateException
	 *	The state of this session is <strong>RESULT</strong>.
	 * @throws IpcBadStateException
	 *	The state of {@code sess} is <strong>READY</strong>.
	 * @throws IpcBadProtocolException
	 *	Additional data in {@code sess} is broken.
	 * @throws IpcUnknownStructException
	 *	{@code sess} keeps at least one unknown IPC structure in the
	 *	additional data array.
	 * @throws IpcStructLayoutMismatchException
	 *	Unexpected IPC structure layout was found in the additional
	 *	data array of {@code sess}.
	 * @throws IpcTooBigDataException
	 *	No more data can not be added to the additional data array.
	 */
	public final void forward(ClientSession sess, int beginIndex,
				  int endIndex)
		throws IpcException
	{
		long src = sess.getSession();

		try {
			long dst = getSession();

			try {
				forward(dst, src, beginIndex, endIndex);
			}
			finally {
				release(dst);
			}
		}
		finally {
			sess.release(src);
		}
	}

	/**
	 * <p>
	 *   Set IPC client session timeout.
	 * </p>
	 *
	 * @param timeout	A {@link TimeSpec} instance which represents
	 *			a new timeout value. {@code null} means an
	 *			infinite timeout.
	 * @throws IpcBadClientSessionException
	 *	This session is already destroyed.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on this session.
	 * @throws IpcShutdownException
	 *	The state of this session is <strong>DISCARD</strong>.
	 *	This exception indicates that this session is no longer
	 *	available.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	public final void setTimeout(TimeSpec timeout) throws IpcException
	{
		long sec, nsec;

		if (timeout == null) {
			sec = -1L;
			nsec = -1L;
		}
		else {
			sec = timeout.getSeconds();
			nsec = timeout.getNanoSeconds();
		}

		long session = getSession();

		try {
			setTimeout(session, sec, nsec);
		}
		finally {
			release(session);
		}
	}

	/**
	 * <p>
	 *   Cancel ongoing IPC service request on this session.
	 * </p>
	 * <p>
	 *   This method terminates ongoing IPC service request on this
	 *   session, and make it throw {@link IpcCanceledException}.
	 * </p>
	 *
	 * @throws IpcBadClientSessionException
	 *	This session is already destroyed.
	 * @throws IpcPermissionDeniedException
	 *	This session is not created with {@link #C_CANCELABLE} flag.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	public final void cancel() throws IpcException
	{
		long session = getSession();

		try {
			cancel(session, false);
		}
		finally {
			release(session);
		}
	}

	/**
	 * <p>
	 *   Discard this session.
	 * </p>
	 * <p>
	 *   This method terminates ongoing IPC service request on this
	 *   session, and make it throw {@link IpcCanceledException}.
	 * </p>
	 * <p>
	 *   After the call of this method, internal state of this session
	 *   will be changed to <strong>DISCARD</strong>. Any further
	 *   IPC service request on this session will get
	 *   {@link IpcShutdownException}.
	 * </p>
	 *
	 * @throws IpcBadClientSessionException
	 *	This session is already destroyed.
	 * @throws IpcPermissionDeniedException
	 *	This session is not created with {@link #C_CANCELABLE} flag.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	public final void discard() throws IpcException
	{
		long session = getSession();

		try {
			cancel(session, true);
		}
		finally {
			release(session);
		}
	}

	/**
	 * <p>
	 *   Destroy this IPC client session explicitly.
	 * </p>
	 * <p>
	 *   This method invalidates this instance immediately.
	 *   After the call of this method, this instance is no longer
	 *   available. If another thread is calling instance method of this
	 *   session, the entity of the IPC client session will be destroyed
	 *   after all method calls are returned.
	 * </p>
	 * <p>
	 *   This method does nothing if this instance is already destroyed.
	 * </p>
	 */
	public void destroy()
	{
		long session;

		synchronized (this) {
			if (_session == SESS_INVALID) {
				// Already destroyed.
				return;
			}

			// Invalidate this session.
			session = _session;
			_session = SESS_INVALID;
		}

		// Release the IPC client session handle.
		release(session);
	}

	/**
	 * <p>
	 *   Ensure that an IPC client session is destroyed when there are no
	 *   more references to this instance.
	 * </p>
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
	 * <p>
	 *   Return the session handle of this instance.
	 *   This method increments the busy counter {@link #_busy} on
	 *   successful return.
	 * </p>
	 *
	 * @return	An IPC client session handle.
	 * @throws IpcBadClientSessionException
	 *	This session is already destroyed.
	 */
	synchronized long getSession() throws IpcException
	{
		long session = _session;

		if (session != SESS_INVALID) {
			_busy++;

			return session;
		}

		throw new IpcBadClientSessionException
			("Client session is already destroyed.");
	}

	/**
	 * <p>
	 *   Release this session held by {@link #getSession()}.
	 * </p>
	 *
	 * @param session	An IPC client session handle.
	 */
	void release(long session)
	{
		synchronized (this) {
			assert _busy > 0 : "Invalid busy counter: " + _busy;
			_busy--;
			if (_busy > 0) {
				// This session is still busy.
				return;
			}

			assert _session == SESS_INVALID;
		}

		destroy(session);
	}

	/**
	 * <p>
	 *   Reset state of this session to <strong>READY</strong>.
	 * </p>
	 *
	 * @param session	An IPC client session handle.
	 * @param name		IPC service name.
	 * @param service	IPC service ID.
	 * @throws IllegalArgumentException
	 *	Invalid IPC service name is specified to {@code name}.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on this session.
	 * @throws IpcShutdownException
	 *	The state of this session is <strong>DISCARD</strong>.
	 *	This exception indicates that this session is no longer
	 *	available.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	native void reset(long session, String name, int service)
		throws IpcException;

	/**
	 * <p>
	 *   Set IPC client session timeout.
	 * </p>
	 *
	 * @param session	An IPC client session handle.
	 * @param sec		The number of seconds of a timeout value.
	 *			Negative value means an infinite timeout.
	 * @param nsec		The number of nanoseconds of a timeout value.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on this session.
	 * @throws IpcShutdownException
	 *	The state of this session is <strong>DISCARD</strong>.
	 *	This exception indicates that this session is no longer
	 *	available.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	native void setTimeout(long session, long sec, long nsec)
		throws IpcException;

	/**
	 * <p>
	 *   Cancel ongoing IPC service request on this session.
	 *</p>
	 * <p>
	 *   If {@code discard} is true, internal state of this session
	 *   will be changed to <strong>DISCARD</strong>. Any further
	 *   IPC service request on this session will get
	 *   {@link IpcShutdownException}.
	 * </p>
	 *
	 * @param session	An IPC client session handle.
	 * @param discard	Determine whether this session should be
	 *			discarded or not.
	 * @throws IpcPermissionDeniedException
	 *	This session is not created with {@link #C_CANCELABLE} flag.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	native void cancel(long session, boolean discard)
		throws IpcException;

	/**
	 * <p>
	 *   Invoke an IPC service on the IPC server.
	 * </p>
	 *
	 * @param session	An IPC client session handle.
	 * @return	Response code received from the IPC server.
	 *		{@link #RESP_FATAL} means a fatal error.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	native int invoke(long session) throws IpcException;

	/**
	 * <p>
	 *   Get an additional data received from the IPC server at the
	 *   specified additional array index.
	 * </p>
	 *
	 * @param session	An IPC client session handle.
	 * @param index		Index of the additional data array.
	 * @return		An additional data received from the IPC
	 *			server.
	 * @throws IpcDataIndexOutOfBoundsException
	 *	{@code index} is greater than or equal to the number of
	 *	additional data array elements.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on this session.
	 * @throws IpcShutdownException
	 *	The state of this session is <strong>DISCARD</strong>.
	 *	This exception indicates that this session is no longer
	 *	available.
	 * @throws IpcBadStateException
	 *	The state of this session is <strong>READY</strong>.
	 */
	private native IpcDataUnit getResponse(long session, int index)
		throws IpcException;

	/**
	 * <p>
	 *   Return the number of additional data array elements.
	 * </p>
	 *
	 * @param session	An IPC client session handle.
	 * @return	The number of additional data array elements received
	 *		by the previous call of {@link #invoke()}.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on this session.
	 * @throws IpcShutdownException
	 *	The state of this session is <strong>DISCARD</strong>.
	 *	This exception indicates that this session is no longer
	 *	available.
	 * @throws IpcBadStateException
	 *	The state of this session is <strong>READY</strong>.
	 * @see	#getResponse(int)
	 */
	private native int getResponseCount(long session) throws IpcException;

	/**
	 * <p>
	 *   Copy additional data received from the IPC server to this
	 *   client session for another IPC service request.
	 * </p>
	 *
	 * @param dst	The client session handle associated with this
	 *		instance.
	 * @param src	The client session handle which keeps additional data.
	 * @param beginIndex	The beginning (inclusive) index.
	 * @param endIndex	The ending (exclusive) index.
	 * @throws IpcDataIndexOutOfBoundsException
	 *	{@code beginIndex} is greater than or equal to the length of
	 *	the additional data array in {@code sess}.
	 * @throws IpcDataIndexOutOfBoundsException
	 *	{@code beginIndex} is greater than {@code endIndex}.
	 * @throws IpcBadClientSessionException
	 *	This session or {@code sess} is already destroyed.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on this session or
	 *	{@code sess}.
	 * @throws IpcShutdownException
	 *	The state of this session or {@code sess} is
	 *	<strong>DISCARD</strong>.
	 * @throws IpcBadStateException
	 *	The state of this session is <strong>RESULT</strong>.
	 * @throws IpcBadStateException
	 *	The state of {@code sess} is <strong>READY</strong>.
	 * @throws IpcBadProtocolException
	 *	Additional data in {@code sess} is broken.
	 * @throws IpcUnknownStructException
	 *	{@code sess} keeps at least one unknown IPC structure in the
	 *	additional data array.
	 * @throws IpcStructLayoutMismatchException
	 *	Unexpected IPC structure layout was found in the additional
	 *	data array of {@code sess}.
	 * @throws IpcTooBigDataException
	 *	No more data can not be added to the additional data array.
	 */
	native void forward(long dst, long src, int beginIndex, int endIndex)
		throws IpcException;

	/**
	 * <p>
	 *   Destroy the specified IPC client session handle.
	 *   The caller must guarantee that this session is no longer used
	 *   by another thread.
	 * </p>
	 *
	 * @param session	An IPC client session handle.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on this session.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	static native void destroy(long session);
}
