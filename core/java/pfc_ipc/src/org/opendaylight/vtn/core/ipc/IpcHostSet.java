/*
 * Copyright (c) 2012-2013 NEC Corporation
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
 *   The {@code IpcHostSet} class provides interfaces to handle IPC
 *   host set.
 * </p>
 * <p>
 *   IPC host set is a set of host addresses on which IPC servers run.
 *   It is used to determine IPC servers to watch IPC events when an IPC
 *   event handler is registered.
 * </p>
 * <p>
 *   IPC host set is identified by the name, which is a human-readable string.
 *   The name if IPC host set must consist of one or more than one alphanumeric
 *   US-ASCII characters, and its length must be within 31. In addition,
 *   it must starts an alphabet.
 * </p>
 * <p>
 *   An instance of {@code IpcHostSet} is just an interface to handle IPC
 *   host set. In other words, the constructor of {@code IpcHostSet} never
 *   creates an IPC host set, and the finalizer never destroys.
 * </p>
 * <p>
 *   Note that all interfaces provided by this class require the IPC event
 *   subsystem is initialized.
 * </p>
 *
 * @since	C10
 * @see		IpcEventSystem#initialize()
 * @see		IpcEventSystem#initialize(IpcEventConfiguration)
 * @see		IpcEventAttribute
 */
public final class IpcHostSet
{
	/**
	 * <p>
	 *   Identifier which represents {@link #add(HostAddress)}.
	 * </p>
	 */
	private final static int	HOSTADDR_ADD = 0;

	/**
	 * <p>
	 *   Identifier which represents {@link #remove(HostAddress)}.
	 * </p>
	 */
	private final static int	HOSTADDR_REMOVE = 1;

	/**
	 * <p>
	 *   Identifier which represents {@link #contains(HostAddress)}.
	 * </p>
	 */
	private final static int	HOSTADDR_CONTAINS = 2;

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
	 *   Create a new IPC host set.
	 * </p>
	 *
	 * @param name	The name of a new IPC host set.
	 * @throws NullPointerException
	 *	{@code name} is {@code null}.
	 * @throws IllegalArgumentException
	 *	{@code name} is invalid.
	 * @throws IpcAlreadyExistsException
	 *	An IPC host set associated with the given name already exists.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled.
	 */
	public static native void create(String name) throws IpcException;

	/**
	 * <p>
	 *   Destroy an IPC host set specified by the given name.
	 * </p>
	 * <p>
	 *   This method always removes the given name from the IPC event
	 *   subsystem as long as it exists. But if the specified IPC host set
	 *   is associated with at least one IPC event handler, the entity of
	 *   the IPC host set will remain until the last event handler which
	 *   uses the IPC host set is removed from the IPC event subsystem.
	 * </p>
	 *
	 * @param name	The name of IPC host set to be destroyed.
	 * @throws NullPointerException
	 *	{@code name} is {@code null}.
	 * @throws IpcNoSuchHostSetException
	 *	IPC host set specified by the name does not exist.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled.
	 */
	public static native void destroy(String name) throws IpcException;

	/**
	 * <p>
	 *   Determine whether an IPC host set specified by the given name
	 *   exists or not.
	 * </p>
	 *
	 * @param name	The name of IPC host set.
	 * @return	{@code true} if the specified IPC host set exists
	 *		in the IPC event subsystem. Otherwise {@code false}.
	 * @throws NullPointerException
	 *	{@code name} is {@code null}.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled.
	 */
	public static native boolean exists(String name) throws IpcException;

	/**
	 * The name of IPC host set.
	 */
	private final String  _name;

	/**
	 * <p>
	 *   Construct an {@code IpcHostSet} instance which handles the
	 *   IPC host set specified by the name.
	 * </p>
	 * <p>
	 *   Note that this constructor never checks whether the given name
	 *   is valid or not.
	 * </p>
	 *
	 * @param name	The name of IPC host set to handle via this instance.
	 * @throws NullPointerException
	 *	{@code name} is {@code null}.
	 */
	public IpcHostSet(String name)
	{
		if (name == null) {
			throw new NullPointerException("name is null.");
		}

		_name = name;
	}

	/**
	 * <p>
	 *  Return the name of IPC host set in this instance.
	 * </p>
	 *
	 * @return	The name of IPC host set.
	 */
	public String getName()
	{
		return _name;
	}

	/**
	 * <p>
	 *   Create an empty IPC host set.
	 * </p>
	 * <p>
	 *   An IPC host set created by this method contains no host address.
	 *   {@link #add(HostAddress)} must be used to add a host address to
	 *   this IPC host set.
	 * </p>
	 *
	 * @throws IllegalArgumentException
	 *	{@code name} is invalid.
	 * @throws IpcAlreadyExistsException
	 *	An IPC host set associated with the given name already exists.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled.
	 */
	public void create() throws IpcException
	{
		create(_name);
	}

	/**
	 * <p>
	 *   Destroy an IPC host set associated with this instance.
	 * </p>
	 * <p>
	 *   This method always removes the name set in this instance from
	 *   the IPC event subsystem as long as it exists. But if the IPC host
	 *   set is associated with at least one IPC event handler, the entity
	 *   of the IPC host set will remain until the last event handler which
	 *   uses the IPC host set is removed from the IPC event subsystem.
	 * </p>
	 *
	 * @throws IpcNoSuchHostSetException
	 *	IPC host set associated with this instance does not exist.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled.
	 */
	public void destroy() throws IpcException
	{
		destroy(_name);
	}

	/**
	 * <p>
	 *   Determine whether an IPC host set associated with this instance
	 *   exists or not.
	 * </p>
	 *
	 * @return	{@code true} if the IPC host set exists in the IPC
	 *		event subsystem. Otherwise {@code false}.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled.
	 */
	public boolean exists() throws IpcException
	{
		return exists(_name);
	}

	/**
	 * <p>
	 *   Add a host address to the IPC host set associated with this
	 *   instance.
	 * </p>
	 * <p>
	 *   If the IPC host set associated with this instance is associated
	 *   with at least one event handler, the specified host address is
	 *   applied to all event handlers associated with the IPC host set,
	 *   and the IPC event subsystem starts event listening against the
	 *   specified host address asynchronously.
	 * </p>
	 *
	 * @param haddr	A {@link HostAddress} instance which represents
	 *		a host address to be added.
	 * @return	{@code true} is returned if the specified host address
	 *		is added. {@code false} is returned if the specified
	 *		host address already exists in the IPC host set.
	 * @throws NullPointerException
	 *	{@code haddr} is {@code null}.
	 * @throws IpcNoSuchHostSetException
	 *	IPC host set specified by the name does not exist.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled.
	 */
	public boolean add(HostAddress haddr) throws IpcException
	{
		return handleHostAddress(_name, HOSTADDR_ADD,
					 haddr.getAddress(),
					 haddr.getScopeId());
	}

	/**
	 * <p>
	 *   Remove a host address from the IPC host set associated with
	 *   this instance.
	 * </p>
	 * <p>
	 *   If the IPC host set associated with this instance is associated
	 *   with at least one event handler, the specified host address is
	 *   applied to all event handlers associated with the IPC host set,
	 *   and the IPC event subsystem discards any event sessions associated
	 *   with the specified host address asynchronously.
	 * </p>
	 *
	 * @param haddr	A {@link HostAddress} instance which represents
	 *		a host address to be removed.
	 * @return	{@code true} is returned if the specified host address
	 *		is removed. {@code false} is returned if the IPC host
	 *		set does not contain the specified host address.
	 * @throws NullPointerException
	 *	{@code haddr} is {@code null}.
	 * @throws IpcNoSuchHostSetException
	 *	IPC host set specified by the name does not exist.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled.
	 */
	public boolean remove(HostAddress haddr) throws IpcException
	{
		return handleHostAddress(_name, HOSTADDR_REMOVE,
					 haddr.getAddress(),
					 haddr.getScopeId());
	}

	/**
	 * <p>
	 *   Determine whether the IPC host set associated with this instance
	 *   contains the specified host address or not.
	 * </p>
	 *
	 * @param haddr	A {@link HostAddress} instance which represents
	 *		a host address to be tested.
	 * @return	{@code true} is returned if the IPC host set contains
	 *		the specified host address. Otherwise {@code false}.
	 * @throws NullPointerException
	 *	{@code haddr} is {@code null}.
	 * @throws IpcNoSuchHostSetException
	 *	IPC host set specified by the name does not exist.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled.
	 */
	public boolean contains(HostAddress haddr) throws IpcException
	{
		return handleHostAddress(_name, HOSTADDR_CONTAINS,
					 haddr.getAddress(),
					 haddr.getScopeId());
	}

	/**
	 * <p>
	 *   Invoke an operation which handles a {@link HostAddress} instance.
	 * </p>
	 *
	 * @param name		The name of IPC host set.
	 * @param op		An operation identifier. Value must be one of
	 *			{@link #HOSTADDR_ADD}, {@link #HOSTADDR_REMOVE},
	 *			and {@link #HOSTADDR_CONTAINS}.
	 * @param rawaddr	Raw IP address in the {@link HostAddress}, or
	 *			{@code null} for local address.
	 * @param scope		A scoped ID in the IPv6 host address, or
	 *			zero for other type.
	 * @return		{@code true} if an operation returned
	 *			successfully. Otherwise {@code false}.
	 * @throws IpcNoSuchHostSetException
	 *	IPC host set specified by the name does not exist.
	 * @throws IpcEventSystemNotReadyException
	 *	The IPC event subsystem is not yet initialized.
	 * @throws IpcCanceledException
	 *	The IPC event subsystem is already disabled.
	 */
	private native boolean handleHostAddress(String name, int op,
						 byte[] rawaddr, int scope)
		throws IpcException;
}
