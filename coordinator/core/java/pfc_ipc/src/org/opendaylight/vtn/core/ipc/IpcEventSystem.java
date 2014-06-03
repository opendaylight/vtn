/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import org.opendaylight.vtn.core.util.HostAddress;

/**
 * <p>
 *   The {@code IpcEventSystem} class is a global manager class of the
 *   IPC event subsystem in the IPC client library.
 * </p>
 * <p>
 *   A single instance returned by {@link #getInstance()} is used to
 *   configure the IPC event subsystem, and is used to add or remove event
 *   handler which receives an event from IPC server.
 * </p>
 *
 * @since	C10
 * @see		IpcEventSystem#initialize()
 * @see		IpcEventSystem#initialize(IpcEventConfiguration)
 */
public final class IpcEventSystem
{
	/**
	 * <p>
	 *   An integer value which indicates that the IPC event listener
	 *   session is not established.
	 *   This value must be equal to the ordinal of
	 *   {@link ChannelState#DOWN}.
	 * </p>
	 */
	private final static int  STATE_DOWN = 0;

	/**
	 * <p>
	 *   An integer value which indicates that the IPC event listener
	 *   session is established.
	 *   This value must be equal to the ordinal of
	 *   {@link ChannelState#UP}.
	 * </p>
	 */
	private final static int  STATE_UP = 1;

	/**
	 * <p>
	 *   An integer value which indicates that the IPC client library is
	 *   trying to establish the IPC event listener session.
	 *   This value must be equal to the ordinal of
	 *   {@link ChannelState#IN_PROGRESS}.
	 * </p>
	 */
	private final static int  STATE_IN_PROGRESS = 2;

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
	 *   Holder class of the manager of the IPC event subsystem.
	 * </p>
	 */
	private final static class IpcEventSystemHolder
	{
		/**
		 * A single global instance of {@link IpcEventSystem}.
		 */
		private final static IpcEventSystem _theInstance =
			new IpcEventSystem();
	}

	/**
	 * <p>
	 *   Return a single global instance of {@link IpcEventSystem}.
	 * </p>
	 *
	 * @return	An {@link IpcEventSystem} instance.
	 */
	public static IpcEventSystem getInstance()
	{
		return IpcEventSystemHolder._theInstance;
	}

	/**
	 * <p>
	 *   Create a single instance of the IPC event subsystem.
	 * </p>
	 */
	private IpcEventSystem()
	{
	}

	/**
	 * <p>
	 *   Initialize the IPC event subsystem with default parameters.
	 * </p>
	 * <p>
	 *   The IPC event subsystem must be initialized before any call of
	 *   methods provided by the IPC event subsystem.
	 * </p>
	 *
	 * @see	#initialize(IpcEventConfiguration)
	 * @throws IpcResourceBusyException
	 *	The IPC event subsystem is already initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled by
	 *	{@link #shutdown()} or {@link #disable()}.
	 * @throws IpcJniException
	 *	The IPC event dispatcher thread could not be attached to the
	 *	Java VM.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 * @see	IpcEventConfiguration
	 */
	public void initialize() throws IpcException
	{
		initialize(null);
	}

	/**
	 * <p>
	 *   Initialize the IPC event subsystem.
	 * </p>
	 * <p>
	 *   The IPC event subsystem must be initialized before any call of
	 *   methods provided by the IPC event subsystem.
	 * </p>
	 *
	 * @param conf	An {@code IpcEventConfiguration} instance which
	 *		contains parameters. If {@code null} is specified,
	 *		the IPC event subsystem is initialized with default
	 *		parameters.
	 * @throws IllegalArgumentException
	 *	{@code conf} contains at least one invalid parameter.
	 * @throws IpcResourceBusyException
	 *	The IPC event subsystem is already initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled by
	 *	{@link #shutdown()} or {@link #disable()}.
	 * @throws IpcJniException
	 *	The IPC event dispatcher thread could not be attached to the
	 *	Java VM.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 * @see	IpcEventConfiguration
	 */
	public native void initialize(IpcEventConfiguration conf)
		throws IpcException;

	/**
	 * <p>
	 *   Start shutdown sequence of the IPC event subsystem.
	 * </p>
	 * <p>
	 *   After successful return of this method, further incoming IPC
	 *   events are discarded. But events already received are delivered
	 *   to event handlers.
	 * </p>
	 *
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	Either this method or {@link #disable()} is already called.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	public native void shutdown() throws IpcException;

	/**
	 * <p>
	 *   Disable the IPC event subsystem permanently.
	 *   Once the IPC event subsystem is disabled by this method,
	 *   it can never be enabled again.
	 * </p>
	 * <p>
	 *   This method calls {@link #shutdown()} if it is not yet called,
	 *   and then frees up all resources held by the IPC event subsystem.
	 * </p>
	 * <ul>
	 *   <li>
	 *     All event handlers and IPC host sets are removed.
	 *   </li>
	 *   <li>
	 *     All event sessions connected to IPC servers are disconnected
	 *     without generating any channel state change event.
	 *   </li>
	 *   <li>
	 *     All pending IPC events are discarded.
	 *   </li>
	 * </ul>
	 * <p>
	 *   This method waits for completion of all threads created by
	 *   the IPC event subsystem at most 10 seconds, including a thread
	 *   which delivers IPC events to IPC event handlers.
	 *   Therefore, this method must not be called from event handler.
	 * </p>
	 * <p>
	 *   It is strongly recommended to call this method when an application
	 *   which uses the IPC event subsystem exits.
	 * </p>
	 *
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	This method is already called.
	 * @throws IpcTimedOutException
	 *	At least one thread created by the IPC event subsystem did not
	 *	complete within 10 seconds.
	 * @throws IpcLibraryException
	 *	The IPC client library returned an error.
	 */
	public native void disable() throws IpcException;

	/**
	 * <p>
	 *   Add an IPC event handler with specifying default IPC event
	 *   attributes.
	 * </p>
	 * <p>
	 *   This method adds an IPC event handler which listens events on
	 *   the IPC server running on the local host.
	 *   If you want to add an IPC event handler which listens events on
	 *   remote IPC server, you needs to create an IPC host set, and
	 *   associate it with the IPC event handler via
	 *   {@link IpcEventAttribute} instance.
	 * </p>
	 * <p>
	 *   The name of the specified event handler, which is used by
	 *   delivery logging, is determined by the class name of the specified
	 *   event handler.
	 * </p>
	 *
	 * @param channel	The IPC channel name associated with the
	 *			target IPC server. If {@code null} is
	 *			specified, the IPC channel name of
	 *			{@link DefaultConnection} is used.
	 * @param handler	An IPC event handler to be added.
	 * @return		An identifier assigned to the specified
	 *			event handler.
	 *			Note that this value must be preserved if
	 *			you want to use {@link #removeHandler(int)}
	 *			or {@link #notifyState(int)}.
	 * @throws NullPointerException
	 *	{@code handler} is {@code null}.
	 * @throws IllegalArgumentException
	 *	An illegal IPC channel name is specified to {@code channel}.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled by
	 *	{@link #shutdown()} or {@link #disable()}.
	 * @see #addHandler(String, IpcEventHandler, IpcEventAttribute)
	 * @see #addHandler(String, IpcEventHandler, IpcEventAttribute, String)
	 */
	public int addHandler(String channel, IpcEventHandler handler)
		throws IpcException
	{
		return addHandler(channel, handler, 0L, createName(handler));
	}

	/**
	 * <p>
	 *   Add an IPC event handler with specifying the specified IPC event
	 *   attributes.
	 * </p>
	 * <p>
	 *   Target IPC servers are determined by the IPC channel specified
	 *   by {@code channel} and an IPC host set specified by
	 *   {@code attr}.
	 * </p>
	 * <ul>
	 *   <li>
	 *     If no IPC host set is specified by {@code attr}, the target IPC
	 *     server is fixed to the server which provides the IPC channel
	 *     {@code channel} on the local host.
	 *   </li>
	 *   <li>
	 *     If an IPC host set is specified by {@code attr}, the IPC client
	 *     library listens events on IPC servers which provide the IPC
	 *     channel {@code channel} on all hosts contained in the IPC host
	 *     set. For instance, if {@code "foo"} is specified to
	 *     {@code channel} and the IPC host set specified by {@code attr}
	 *     contains 3 host addresses, local address
	 *     (see {@link HostAddress#getLocalAddress()}),
	 *     192.168.10.1, and 192.168.10.2, the specified
	 *     IPC handler will listen events on the following IPC servers.
	 *     <ul>
	 *       <li>
	 *         IPC server which provides the IPC channel {@code "foo"}
	 *         on the local host.
	 *       </li>
	 *       <li>
	 *         IPC server which provides the IPC channel {@code "foo"}
	 *         on 192.168.10.1.
	 *       </li>
	 *       <li>
	 *         IPC server which provides the IPC channel {@code "foo"}
	 *         on 192.168.10.2.
	 *       </li>
	 *     </ul>
	 *   </li>
	 * </ul>
	 * <p>
	 *   The name of the specified event handler, which is used by
	 *   delivery logging, is determined by the class name of the specified
	 *   event handler.
	 * </p>
	 * <p>
	 *   This method never modifies the IPC event attributes specified by
	 *   {@code attr}. So it can be used to add another IPC event handler.
	 * </p>
	 *
	 * @param channel	The IPC channel name associated with the
	 *			target IPC server. If {@code null} is
	 *			specified, the IPC channel name of
	 *			{@link DefaultConnection} is used.
	 * @param handler	An IPC event handler to be added.
	 * @param attr		IPC event attributes which determine behavior
	 *			of {@code handler}.
	 * @return		An identifier assigned to the specified
	 *			event handler.
	 *			Note that this value must be preserved if
	 *			you want to use {@link #removeHandler(int)}
	 *			or {@link #notifyState(int)}.
	 * @throws NullPointerException
	 *	Either {@code handler} or {@code attr} is {@code null}.
	 * @throws IllegalArgumentException
	 *	An illegal IPC channel name is specified to {@code channel}.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled by
	 *	{@link #shutdown()} or {@link #disable()}.
	 * @see #addHandler(String, IpcEventHandler, IpcEventAttribute, String)
	 */
	public int addHandler(String channel, IpcEventHandler handler,
			      IpcEventAttribute attr)
		throws IpcException
	{
		return addHandler(channel, handler, attr.getHandle(),
				  createName(handler));
	}

	/**
	 * <p>
	 *   Add an IPC event handler with specifying the specified IPC event
	 *   attributes.
	 * </p>
	 * <p>
	 *   Target IPC servers are determined by the IPC channel specified
	 *   by {@code channel} and an IPC host set specified by
	 *   {@code attr}.
	 * </p>
	 * <ul>
	 *   <li>
	 *     If no IPC host set is specified by {@code attr}, the target IPC
	 *     server is fixed to the server which provides the IPC channel
	 *     {@code channel} on the local host.
	 *   </li>
	 *   <li>
	 *     If an IPC host set is specified by {@code attr}, the IPC client
	 *     library listens events on IPC servers which provide the IPC
	 *     channel {@code channel} on all hosts contained in the IPC host
	 *     set. For instance, if {@code "foo"} is specified to
	 *     {@code channel} and the IPC host set specified by {@code attr}
	 *     contains 3 host addresses, local address
	 *     (see {@link HostAddress#getLocalAddress()}),
	 *     192.168.10.1, and 192.168.10.2, the specified
	 *     IPC handler will listen events on the following IPC servers.
	 *     <ul>
	 *       <li>
	 *         IPC server which provides the IPC channel {@code "foo"}
	 *         on the local host.
	 *       </li>
	 *       <li>
	 *         IPC server which provides the IPC channel {@code "foo"}
	 *         on 192.168.10.1.
	 *       </li>
	 *       <li>
	 *         IPC server which provides the IPC channel {@code "foo"}
	 *         on 192.168.10.2.
	 *       </li>
	 *     </ul>
	 *   </li>
	 * </ul>
	 * <p>
	 *   {@code name} specifies the name of the IPC event handler,
	 *   which is used by delivery logging.
	 * </p>
	 * <p>
	 *   This method never modifies the IPC event attributes specified by
	 *   {@code attr}. So it can be used to add another IPC event handler.
	 * </p>
	 *
	 * @param channel	The IPC channel name associated with the
	 *			target IPC server. If {@code null} is
	 *			specified, the IPC channel name of
	 *			{@link DefaultConnection} is used.
	 * @param handler	An IPC event handler to be added.
	 * @param attr		IPC event attributes which determine behavior
	 *			of {@code handler}.
	 * @param name		The name of the IPC event handler.
	 *			If {@code null} is specified, the name is
	 *			determined by the class name of
	 *			{@code handler}.
	 *			Currently this is used only for logging.
	 * @return		An identifier assigned to the specified
	 *			event handler.
	 *			Note that this value must be preserved if
	 *			you want to use {@link #removeHandler(int)}
	 *			or {@link #notifyState(int)}.
	 * @throws NullPointerException
	 *	Either {@code handler} or {@code attr} is {@code null}.
	 * @throws IllegalArgumentException
	 *	An illegal IPC channel name is specified to {@code channel}.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled by
	 *	{@link #shutdown()} or {@link #disable()}.
	 */
	public int addHandler(String channel, IpcEventHandler handler,
			      IpcEventAttribute attr, String name)
		throws IpcException
	{
		if (name == null) {
			name = createName(handler);
		}

		return addHandler(channel, handler, attr.getHandle(), name);
	}

	/**
	 * <p>
	 *   Remove an IPC event handler specified by the given ID.
	 * </p>
	 * <p>
	 *   {@code id} must be an identifier assigned to the IPC event
	 *   handler by {@link #addHandler(String, IpcEventHandler)} or
	 *   its variants. If the IPC event listener session becomes idle as
	 *   a result of removal, it will be discarded.
	 * </p>
	 * <p>
	 *   This method always waits for the completion of the specified IPC
	 *   event handler within 5 seconds. So this method must not be called
	 *   by the IPC event handler.
	 * </p>
	 *
	 * @param id	Identifier of the IPC event handler to be removed.
	 * @throws IpcNoSuchEventHandlerException
	 *	The IPC event handler specified by {@code id} does not exist.
	 * @throws IpcTimedOutException
	 *	The IPC event handler specified by {@code id} did not complete
	 *	within 5 seconds. In this case the handler is invalidated
	 *	immediately, and will be removed when it completes.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled by
	 *	{@link #disable()}.
	 * @see #addHandler(String, IpcEventHandler)
	 * @see #addHandler(String, IpcEventHandler, IpcEventAttribute)
	 * @see #addHandler(String, IpcEventHandler, IpcEventAttribute, String)
	 */
	public native void removeHandler(int id) throws IpcException;

	/**
	 * <p>
	 *   Enable or disable the stack trace dump on the IPC event dispatcher
	 *   thread.
	 * </p>
	 * <p>
	 *   The event dispatcher thread catches a {@code Throwable} thrown
	 *   by {@link IpcEventHandler#eventReceived(IpcEvent)}.
	 *   If the stack trace dump is enabled, the event dispatched thread
	 *   prints the {@code Throwable} and its backtrace to the standard
	 *   error output stream.
	 * </p>
	 * <p>
	 *   If the stack trace dump is disabled, the event dispatcher thread
	 *   simply clears uncaught {@code Throwable}.
	 *   This is the default behavior.
	 * </p>
	 * <p>
	 *   Note that this method can be called even the IPC event subsystem
	 *   is not initialized.
	 * </p>
	 *
	 * @param enabled	Enable the stack trace dump if {@code true}.
	 *			Disable it if {@code false}.
	 */
	public native void setStackTraceEnabled(boolean enabled);

	/**
	 * <p>
	 *   Return the state of the IPC event listener session associated
	 *   with the IPC server on the local host.
	 * </p>
	 * <p>
	 *   This method returns the state of the IPC event listener session
	 *   which connects to the local host.
	 *   {@link #getChannelState(String, HostAddress)} must be used to
	 *   check the state of remote session.
	 * </p>
	 * <p>
	 *   Note that this method requires at least one event handler which
	 *   listens events on the IPC server specified by {@code channel}.
	 * </p>
	 *
	 * @param channel	The IPC channel name associated with the
	 *			target IPC server. If {@code null} is
	 *			specified, the IPC channel name of
	 *			{@link DefaultConnection} is used.
	 * @return		A {@link ChannelState} instance which
	 *			represents the current state of the IPC event
	 *			listener session which listens events on the
	 *			specified IPC server.
	 * @throws IpcNoSuchEventHandlerException
	 *	No IPC event handler which listens events on the specified
	 *      IPC server was found.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled by
	 *	{@link #shutdown()} or {@link #disable()}.
	 * @see #addHandler(String, IpcEventHandler)
	 * @see #addHandler(String, IpcEventHandler, IpcEventAttribute)
	 * @see #addHandler(String, IpcEventHandler, IpcEventAttribute, String)
	 */
	public ChannelState getChannelState(String channel) throws IpcException
	{
		int state = getChannelState(channel, null, 0);

		return ChannelState.values()[state];
	}

	/**
	 * <p>
	 *   Return the state of the IPC event listener session associated
	 *   with the IPC server specified by the channel name and the
	 *   host address.
	 * </p>
	 * <p>
	 *   The target IPC event listener session is specified by the IPC
	 *   channel name and the host address on which the IPC server runs.
	 *   Note that this method requires at least one event handler which
	 *   listens events on the IPC server specified by {@code channel}
	 *   and {@code haddr}.
	 * </p>
	 *
	 * @param channel	The IPC channel name associated with the
	 *			target IPC server. If {@code null} is
	 *			specified, the IPC channel name of
	 *			{@link DefaultConnection} is used.
	 * @param haddr		A {@link HostAddress} instance which specifies
	 *			the host address on which the IPC server runs.
	 * @return		A {@link ChannelState} instance which
	 *			represents the current state of the IPC event
	 *			listener session which listens events on the
	 *			specified IPC server.
	 * @throws NullPointerException
	 *	{@code haddr} is {@code null}.
	 * @throws IpcNoSuchEventHandlerException
	 *	No IPC event handler which listens events on the specified
	 *      IPC server was found.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled by
	 *	{@link #shutdown()} or {@link #disable()}.
	 * @see #addHandler(String, IpcEventHandler)
	 * @see #addHandler(String, IpcEventHandler, IpcEventAttribute)
	 * @see #addHandler(String, IpcEventHandler, IpcEventAttribute, String)
	 */
	public ChannelState getChannelState(String channel, HostAddress haddr)
		throws IpcException
	{
		int state = getChannelState(channel, haddr.getAddress(),
					    haddr.getScopeId());

		return ChannelState.values()[state];
	}

	/**
	 * <p>
	 *   Generate a channel state notification event and deliver it to the
	 *   specified IPC event handler.
	 * </p>
	 * <p>
	 *   {@code id} must be an identifier of the IPC event handler, and
	 *   the IPC event handler specified by {@code id} must be configured
	 *   to listen channel notification events.
	 * </p>
	 * <p>
	 *   This method generates a channel state notification event per
	 *   an IPC event listener session. For example, if the IPC host set
	 *   associated with the specified IPC event handler contains 10
	 *   host addresses, this method will generate 10 channel state
	 *   notification events.
	 * </p>
	 *
	 * @param id	Identifier of the IPC event handler.
	 * @throws IpcNoSuchEventHandlerException
	 *	The IPC event handler specified by {@code id} does not exist.
	 * @throws IpcNoSuchEventHandlerException
	 *	The IPC event handler specified by {@code id} does not listen
	 *	channel state notification events.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled by
	 *	{@link #shutdown()} or {@link #disable()}.
	 * @see #addHandler(String, IpcEventHandler)
	 * @see #addHandler(String, IpcEventHandler, IpcEventAttribute)
	 * @see #addHandler(String, IpcEventHandler, IpcEventAttribute, String)
	 * @see	IpcEvent
	 * @see	IpcEventAttribute#addTarget(String)
	 * @see	IpcEventAttribute#addTarget(String, IpcEventMask)
	 */
	public native void notifyState(int id) throws IpcException;

	/**
	 * <p>
	 *   Enable or disable auto-cancellation of IPC client session.
	 * </p>
	 * <p>
	 *   When an event listener session is disconnected unexpectedly,
	 *   the IPC event subsystem check whether auto-cancellation of
	 *   IPC client session is enabled or not. If enabled, the IPC event
	 *   subsystem calls {@link ClientSession#cancel()} for each IPC
	 *   client session associated with the same IPC server as disconnected
	 *   listener session.
	 * </p>
	 * <ul>
	 *   <li>
	 *     Auto-cancellation is disabled by default.
	 *   </li>
	 *   <li>
	 *     This method can be called even if the IPC event subsystem is
	 *     not initialized.
	 *   </li>
	 * </ul>
	 *
	 * @param value  Auto-cancellation of IPC client session is enabled
	 *               if {@code true} is specified.
	 *               It is disabled if {@code false} is specified.
	 * @since  C14
	 */
	public native void setAutoCancelEnabled(boolean value);

	/**
	 * <p>
	 *   Determine whether auto-cancellation of IPC client session is
	 *   enabled or not.
	 * </p>
	 *
	 * @return  {@code true} if enabled. {@code false} if disabled.
	 * @see  #setAutoCancelEnabled(boolean)
	 * @since  C14
	 */
	public native boolean isAutoCancelEnabled();

	/**
	 * <p>
	 *   Add an IPC event handler with specifying the specified IPC event
	 *   attributes.
	 * </p>
	 *
	 * @param channel	The IPC channel name associated with the
	 *			target IPC server. If {@code null} is
	 *			specified, the IPC channel name of
	 *			{@link DefaultConnection} is used.
	 * @param handler	An IPC event handler to be added.
	 * @param ahandle	A handle for an IPC event attributes which
	 *			determines behavior of {@code handler}.
	 * @param name		The name of the IPC event handler.
	 *			Currently this is used only for logging.
	 * @return		An identifier assigned to the specified
	 *			event handler.
	 *			Note that this value must be preserved if
	 *			you want to use {@link #removeHandler(int)}
	 *			or {@link #notifyState(int)}.
	 * @throws NullPointerException
	 *	{@code handler} is {@code null}.
	 * @throws IllegalArgumentException
	 *	An illegal IPC channel name is specified to {@code channel}.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	Either this method or {@link #shutdown()} is already called.
	 */
	private synchronized native int addHandler(String channel,
						   IpcEventHandler handler,
						   long ahandle, String name)
		throws IpcException;

	/**
	 * <p>
	 *   Create the name of the specified IPC handler.
	 * </p>
	 *
	 * @param handler	The IPC event handler.
	 * @return		The name of the specified handler.
	 * @throws NullPointerException
	 *	{@code handler} is {@code null}.
	 */
	private String createName(IpcEventHandler handler)
	{
		// Determine handler's name from its class name.
		String name = handler.getClass().getName();
		int idx = name.lastIndexOf('.');
		if (idx != -1) {
			name = name.substring(idx + 1);
		}

		return name;
	}

	/**
	 * <p>
	 *   Return the state of the IPC event listener session associated
	 *   with the IPC server specified by the channel name and the
	 *   host address.
	 * </p>
	 *
	 * @param channel	The IPC channel name associated with the
	 *			target IPC server. If {@code null} is
	 *			specified, the IPC channel name of
	 *			{@link DefaultConnection} is used.
	 * @param rawaddr	Raw IP address in the {@link HostAddress}, or
	 *			{@code null} for local address.
	 * @param scope		A scoped ID in the IPv6 host address, or
	 *			zero for other type.
	 * @return		An integer which represents the state of
	 *			the IPC event listener session.
	 * @throws IpcNoSuchEventHandlerException
	 *	No IPC event handler associated with the specified IPC server
	 *	is added.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled by
	 *	{@link #shutdown()} or {@link #disable()}.
	 */
	private native int getChannelState(String channel, byte[] rawaddr,
					   int scope)
		throws IpcException;
}
