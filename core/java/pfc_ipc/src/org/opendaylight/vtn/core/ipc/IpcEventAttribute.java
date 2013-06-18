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
 *   The {@code IpcEventAttribute} class provides an interface to define
 *   attributes of IPC event handler.
 * </p>
 * <p>
 *   The following attributes can be defined to an IPC event handler via
 *   {@code IpcEventAttribute} instance.
 * </p>
 * <dl>
 *   <dt>IPC host set</dt>
 *   <dd>
 *     Specify the name of IPC host set ({@link IpcHostSet}) which contains
 *     host addresses of IPC servers.
 *     The IPC client library tries to establish the event listener session
 *     to all hosts in the specified IPC host set.
 *     <ul>
 *       <li>
 *         No IPC host set is defined by default. In this case, the target
 *         host of the event listener session is fixed to local host only.
 *       </li>
 *     </ul>
 *   </dd>
 *
 *   <dt>Target event set</dt>
 *   <dd>
 *     The target event is specified by a pair of IPC service name and
 *     IPC event mask ({@link IpcEventMask}), and the target event set is
 *     a set of target events. Only IPC events that match at least one target
 *     event in the target event set are delivered to IPC event handler.
 *     <ul>
 *       <li>
 *         Channel state change event can be specified by specifying
 *         {@code null} as IPC service name. See description on
 *         {@link IpcEvent} for details about channel state change event.
 *       </li>
 *       <li>
 *         No target event is defined by default. In this case, all received
 *         IPC events, including server state change event, are delivered to
 *         IPC event handler.
 *       </li>
 *     </ul>
 *   </dd>
 *
 *   <dt>Priority</dt>
 *   <dd>
 *     Priority is an integer which determines order of IPC event handlers
 *     to be invoked. When an IPC event is delivered to more than one IPC
 *     event handlers, it is delivered in ascending handler's priority order.
 *     <ul>
 *       <li>
 *         Note that a priority value is treated as unsigned 32-bit integer.
 *       </li>
 *       <li>
 *         Default priority value is defined by {@link #DEFAULT_PRIORITY}.
 *       </li>
 *     </ul>
 *   </dd>
 *
 *   <dt>Delivery logging</dt>
 *   <dd>
 *     Determine whether the IPC client library should record event delivery
 *     logging when it delivers to IPC event handler.
 *     <ul>
 *       <li>
 *         Delivery logging is disabled by default.
 *       </li>
 *     </ul>
 *   </dd>
 * </dl>
 * <p>
 *   Note that this class is not synchronized.
 *   Concurrent access to an {@code IpcEventMask} from multiple threads
 *   results in undefined behavior.
 * </p>
 *
 * @since	C10
 * @see		IpcEvent
 */
public final class IpcEventAttribute
{
	/**
	 * Default priority value.
	 */
	public final static int  DEFAULT_PRIORITY = 100;

	/**
	 * Load native library.
	 */
	static {
		ClientLibrary.load();
	}

	/**
	 * IPC event attributes handle.
	 */
	private final long  _handle;

	/**
	 * Construct a new IPC event attributes.
	 * All attributes are initialized with default value.
	 */
	public IpcEventAttribute()
	{
		_handle = createHandle();
	}

	/**
	 * Return the name of IPC host set in this IPC event attributes.
	 *
	 * @return	The name of IPC host set, or {@code null} if no IPC
	 *		host set is defined in this instance.
	 */
	public String getHostSet()
	{
		return getHostSet(_handle);
	}

	/**
	 * <p>
	 *   Set IPC host set to this IPC event attributes.
	 * </p>
	 * <p>
	 *   The IPC client library tries to establish the event listener
	 *   session to all hosts in the specified IPC host set.
	 *   If {@code name} is {@code null}, the IPC host set in this IPC
	 *   event attributes is reset to initial state.
	 * </p>
	 *
	 * @param name	The name of IPC host set, or {@code null}.
	 * @throws IpcNoSuchHostSetException
	 *	IPC host set specified by the name does not exist.
	 */
	public void setHostSet(String name) throws IpcException
	{
		setHostSet(_handle, name);
	}

	/**
	 * <p>
	 *   Get an IPC event mask associated with the specified IPC service
	 *   name in the target event set.
	 * </p>
	 * <p>
	 *   {@code service} specifies the name of IPC service name.
	 *   {@code null} means the server state change event.
	 * </p>
	 * <ul>
	 *   <li>
	 *     If no target event is set in this IPC event attributes, which
	 *     is initial state, all IPC events are delivered to IPC event
	 *     handlers associated with this IPC event attributes. So this
	 *     method returns an {@link IpcEventMask} instance which contains
	 *     all supported IPC event types.
	 *   </li>
	 *   <li>
	 *     If at least one target event is set in this IPC event
	 *     attributes, and the specified IPC service name does not exist
	 *     in the target event set, this method returns an empty
	 *     {@code IpcEventMask} instance.
	 *   </li>
	 * </ul>
	 *
	 * @param service	The IPC service name to be tested,
	 *			or {@code null}.
	 * @return		An {@link IpcEventMask} instance associated
	 *			with {@code service} in the target event set.
	 */
	public IpcEventMask getTarget(String service)
	{
		return new IpcEventMask(getTarget(_handle, service));
	}

	/**
	 * <p>
	 *   Add a target event into the target event set in this IPC event
	 *   attributes.
	 * </p>
	 * <p>
	 *   {@code service} specifies the name of IPC service name.
	 *   {@code null} means the server state change event.
	 *   This method associates an IPC event mask which contains all
	 *   supported IPC event types with the IPC service name specified
	 *   by {@code service}. Therefore, all IPC events generated by the
	 *   IPC service specified by {@code service} will be delivered
	 *   to all IPC event handlers associated with this IPC event
	 *   attributes.
	 * </p>
	 *
	 * @param service	The IPC service name to be tested,
	 *			or {@code null}.
	 * @throws IllegalArgumentException
	 *	An invalid IPC service name is specified to {@code service}.
	 */
	public void addTarget(String service)
	{
		addTarget(_handle, service, IpcEventMask.MASK_FILLED);
	}

	/**
	 * <p>
	 *   Add a target event into the target event set in this IPC event
	 *   attributes.
	 * </p>
	 * <p>
	 *   {@code service} specifies the name of IPC service name.
	 *   {@code null} means the server state change event.
	 *   If {@code service} already exists in the target event set,
	 *   an IPC event mask specified by {@code mask} is merged to the value
	 *   in the target event set.
	 * </p>
	 *
	 * @param service	The IPC service name to be tested,
	 *			or {@code null}.
	 * @param mask		An {@link IpcEventMask} object to be associated
	 *			with {@code service} in the target event set.
	 *			If {@code null} is set, all supported IPC event
	 *			types are set in the target event set.
	 * @throws NullPointerException
	 *	{@code mask} is null.
	 * @throws IllegalArgumentException
	 *	An invalid IPC service name is specified to {@code service}.
	 * @throws IllegalArgumentException
	 *	{@code mask} is empty, which means it contains no IPC event
	 *	type.
	 */
	public void addTarget(String service, IpcEventMask mask)
	{
		addTarget(_handle, service, mask.getMask());
	}

	/**
	 * <p>
	 *   Reset the target event set in this IPC event attributes to
	 *   initial state.
	 * </p>
	 * <p>
	 *   This method invalidates all target events added by
	 *   {@link #addTarget(String)} or
	 *   {@link #addTarget(String, IpcEventMask)}
	 * </p>
	 */
	public void resetTarget()
	{
		resetTarget(_handle);
	}

	/**
	 * Return an IPC event handler's priority value set in this IPC event
	 * attributes.
	 *
	 * @return	A priority value which determines order of IPC event
	 *		handler to be invoked.
	 *		Note that returned value must be treated as unsigned
	 *		32-bit integer.
	 */
	public int getPriority()
	{
		return getPriority(_handle);
	}

	/**
	 * Set an IPC event handler's priority value to this IPC event
	 * attributes.
	 *
	 * @param priority	An integer value which determines order of
	 *			IPC event handler to be invoked.
	 *			Note that the specified value is always treated
	 *			as unsigned 32-bit integer.
	 */
	public void setPriority(int priority)
	{
		setPriority(_handle, priority);
	}

	/**
	 * Determine whether the event delivery logging is enabled or not.
	 *
	 * @return	{@code true} is returned if the event delivery logging
	 *		is enabled in this IPC event attributes.
	 *		Otherwise {@code false}.
	 */
	public boolean isLogEnabled()
	{
		return isLogEnabled(_handle);
	}

	/**
	 * <p>
	 *   Enable or disable the event delivery logging in this IPC event
	 *   attributes.
	 * </p>
	 * <p>
	 *   Note that event delivery logs are recorded to the IPC client
	 *   internal log. So the IPC client internal logging must be enabled
	 *   by {@link ClientLibrary#setLogEnabled(boolean)} beforehand
	 *   if you need the event delivery logging.
	 * </p>
	 *
	 * @param enabled	{@code true} means the event delivery logging
	 *			should be enabled. {@code false} means it
	 *			should be disabled.
	 * @see	ClientLibrary#setLogEnabled(boolean)
	 */
	public void setLogEnabled(boolean enabled)
	{
		setLogEnabled(_handle, enabled);
	}

	/**
	 * Return a handle for this IPC event attributes.
	 *
	 * @return	A handle for this IPC event attributes.
	 */
	long getHandle()
	{
		return _handle;
	}

	/**
	 * Finalize this IPC event attributes.
	 */
	@Override
	protected void finalize() throws Throwable
	{
		finalize(_handle);
	}

	/**
	 * Create a new IPC event attributes handle.
	 *
	 * @return	A new IPC event attribute handle.
	 */
	private static native long createHandle();

	/**
	 * Return the name of IPC host set in this IPC event attributes.
	 *
	 * @param handle	A handle for IPC event attributes.
	 * @return	The name of IPC host set, or {@code null} if no IPC
	 *		host set is defined in this instance.
	 */
	private native String getHostSet(long handle);

	/**
	 * <p>
	 *   Set IPC host set to this IPC event attributes.
	 * </p>
	 * <p>
	 *   The IPC client library tries to establish the event listener
	 *   session to all hosts in the specified IPC host set.
	 *   If {@code name} is {@code null}, the IPC host set in this IPC
	 *   event attributes is reset to initial state.
	 * </p>
	 *
	 * @param handle	A handle for IPC event attributes.
	 * @param name	The name of IPC host set, or {@code null}.
	 * @throws IpcNoSuchHostSetException
	 *	IPC host set specified by the name does not exist.
	 */
	private native void setHostSet(long handle, String name)
		throws IpcException;

	/**
	 * Get an IPC event mask associated with the specified IPC service
	 * name in the target event set.
	 *
	 * @param handle	A handle for IPC event attributes.
	 * @param service	The IPC service name to be tested,
	 *			or {@code null}.
	 * @return		An IPC event mask value associated with
	 *			{@code service} in the target event set.
	 */
	private native long getTarget(long handle, String service);

	/**
	 * Add a target event into the target event set in this IPC event
	 * attributes.
	 *
	 * @param handle	A handle for IPC event attributes.
	 * @param service	The IPC service name to be tested,
	 *			or {@code null}.
	 * @param mask		An IPC event mask value.
	 * @throws IllegalArgumentException
	 *	An invalid IPC service name is specified to {@code service}.
	 * @throws IllegalArgumentException
	 *	{@code mask} is zero.
	 */
	private native void addTarget(long handle, String service, long mask);

	/**
	 * Reset the target event set in this IPC event attributes to
	 * initial state.
	 *
	 * @param handle	A handle for IPC event attributes.
	 */
	private native void resetTarget(long handle);

	/**
	 * Return an IPC event handler's priority value set in this IPC event
	 * attributes.
	 *
	 * @param handle	A handle for IPC event attributes.
	 * @return		A priority value which determines order of
	 *			IPC event handler to be invoked.
	 */
	private native int getPriority(long handle);

	/**
	 * Set an IPC event handler's priority value to this IPC event
	 * attributes.
	 *
	 * @param handle	A handle for IPC event attributes.
	 * @param priority	An integer value which determines order of
	 *			IPC event handler to be invoked.
	 */
	private native void setPriority(long handle, int priority);

	/**
	 * Determine whether the event delivery logging is enabled or not.
	 *
	 * @param handle	A handle for IPC event attributes.
	 * @return		{@code true} is returned if the event delivery
	 *			logging is enabled in this IPC event
	 *			attributes. Otherwise {@code false}.
	 */
	private native boolean isLogEnabled(long handle);

	/**
	 * Enable or disable the event delivery logging in this IPC event
	 * attributes.
	 *
	 * @param handle	A handle for IPC event attributes.
	 * @param enabled	{@code true} means the event delivery logging
	 *			should be enabled. {@code false} means it
	 *			should be disabled.
	 */
	private native void setLogEnabled(long handle, boolean enabled);

	/**
	 * Finalize this IPC event attributes.
	 *
	 * @param handle	A handle for IPC event attributes.
	 */
	private native void finalize(long handle);
}
