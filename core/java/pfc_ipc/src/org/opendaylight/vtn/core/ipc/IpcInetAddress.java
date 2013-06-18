/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import java.net.InetAddress;
import java.net.Inet4Address;
import java.net.Inet6Address;
import org.opendaylight.vtn.core.util.HostAddress;

/**
 * <p>
 *   The {@code IpcInetAddress} class is a base class of IPC data unit which
 *   represents an Internet Protocol (IP) address.
 * </p>
 * <p>
 *   To construct IPC data unit which represents an IP address,
 *   a data unit object must be created by {@link #create(InetAddress)}.
 * </p>
 *
 * @since	C10
 */
public abstract class IpcInetAddress extends IpcDataUnit
{
	/**
	 * The number of bytes in an IPv6 address.
	 */
	private final static int  ADDRLEN_INET6 = 16;

	/**
	 * The IP address.
	 */
	final InetAddress  _value;

	/**
	 * <p>
	 *   Create a new IPC data unit indicated by the specified IP address.
	 * </p>
	 * <ul>
	 *   <li>
	 *     If an {@code Inet4Address} object is specified, this method
	 *     creates a new {@link IpcInet4Address} object.
	 *   </li>
	 *   <li>
	 *     If an {@code Inet6Address} object is specified, this method
	 *     creates a new {@link IpcInet6Address} object.
	 *   </li>
	 * </ul>
	 *
	 * @param value		An IP address.
	 * @return		A new IPC data unit object which represents
	 *			the specified IP address.
	 * @throws NullPointerException
	 *	{@code value} is null.
	 */
	public final static IpcInetAddress create(InetAddress value)
	{
		if (value instanceof Inet4Address) {
			return new IpcInet4Address(value);
		}
		if (value instanceof Inet6Address) {
			return new IpcInet6Address(value);
		}

		if (value == null) {
			throw new NullPointerException("value is null.");
		}

		// This should never happen.
		throw new IllegalArgumentException("Unexpected value: " +
						   value);
	}

	/**
	 * Create a new IPC data unit indicated by the specified raw IP
	 * address.
	 *
	 * @param addr		A raw IP address in network byte order.
	 * @return		A new IPC data unit object which represents
	 *			the specified raw IP address.
	 * @throws NullPointerException
	 *	{@code addr} is null.
	 * @throws IllegalArgumentException
	 *	Unexpected raw IP address is specified.
	 */
	public final static IpcInetAddress create(byte[] addr)
	{
		// InetAddress.getByAddress() returns Inet4Address if the byte
		// array contains an IPv4 mapped IPv6 address. So we must use
		// Inet6Address.getByAddress() explicitly if the specified
		// array contains an IPv6 address.
		int len = addr.length;

		try {
			InetAddress iaddr = (len == ADDRLEN_INET6)
				? Inet6Address.getByAddress(null, addr, -1)
				: InetAddress.getByAddress(addr);

			return create(iaddr);
		}
		catch (Exception e) {
			throw new IllegalArgumentException
				("Failed to create IpcInetAddress.", e);
		}
	}

	/**
	 * Construct an IP address.
	 *
	 * @param value		A value for a new object.
	 */
	IpcInetAddress(InetAddress value)
	{
		assert value != null: "null value";
		_value = value;
	}

	/**
	 * Get the value of this object.
	 *
	 * @return	An {@code InetAddress} object which represents this
	 *		value.
	 */
	public final InetAddress getValue()
	{
		return _value;
	}

	/**
	 * Return a {@code String} which represents this value.
	 *
	 * Note that a string returned by this method may differ from
	 * a string returned by {@link InetAddress#toString()} or
	 * {@link InetAddress#getHostAddress()}.
	 *
	 * @return	A {@code String} which represents an IP address
	 *		contained by this object.
	 */
	@Override
	public final String toString()
	{
		return HostAddress.getAddress(_value);
	}

	/**
	 * Return a hash code of this value.
	 *
	 * @return	A hash code of this value.
	 */
	@Override
	public final int hashCode()
	{
		return _value.hashCode();
	}

	/**
	 * Determine whether the specified object is identical to this object.
	 *
	 * @param obj	An object to be compared.
	 * @return	True is returned if the specified object is identical
	 *		to this object. Otherwise false is returned.
	 */
	@Override
	public final boolean equals(Object obj)
	{
		if (obj instanceof IpcInetAddress) {
			return _value.equals(((IpcInetAddress)obj)._value);
		}

		return false;
	}

	/**
	 * Append this data to the end of the additional data in the
	 * specified IPC client session
	 *
	 * Note that this method must be called by an internal method of
	 * {@link ClientSession}.
	 *
	 * @param session	An IPC client session handle.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on the specified
	 *	session.
	 * @throws IpcShutdownException
	 *	The state of the specified session is <strong>DISCARD</strong>.
	 *	This exception indicates that the specified session is no
	 *	longer available.
	 * @throws IpcBadStateException
	 *	The state of the specified session is <strong>RESULT</strong>.
	 * @throws IpcTooBigDataException
	 *	No more additional data can not be added.
	 */
	@Override
	final void addClientOutput(long session) throws IpcException
	{
		addClientOutput(session, _value.getAddress());
	}

	/**
	 * Append this data to the end of the additional data in the
	 * specified IPC client session
	 *
	 * @param session	An IPC client session handle.
	 * @param addr		A {@code byte} array which contains raw IP
	 *			address.
	 * @throws IpcResourceBusyException
	 *	Another IPC service request is being issued on the specified
	 *	session.
	 * @throws IpcShutdownException
	 *	The state of the specified session is <strong>DISCARD</strong>.
	 *	This exception indicates that the specified session is no
	 *	longer available.
	 * @throws IpcBadStateException
	 *	The state of the specified session is <strong>RESULT</strong>.
	 * @throws IpcTooBigDataException
	 *	No more additional data can not be added.
	 */
	abstract void addClientOutput(long session, byte[] addr)
		throws IpcException;

	/**
	 * Set this value to the specified IPC structure field.
	 * This method is called by {@link IpcStruct#set(String, IpcDataUnit)}.
	 *
	 * @param target	The target {@link IpcStruct} instance.
	 * @param name		The name of the target field in {@code target}.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a valid field name.
	 * @throws IllegalArgumentException
	 *	{@code name} is an array field name.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @see	IpcStruct#set(String, IpcDataUnit)
	 */
	@Override
	final void setStruct(IpcStruct target, String name)
	{
		target.set(name, _value);
	}

	/**
	 * Set this value into an array element at the specified IPC
	 * structure field.
	 * This method is called by
	 * {@link IpcStruct#set(String, int, IpcDataUnit)}.
	 *
	 * @param target	The target {@link IpcStruct} instance.
	 * @param name		The name of the target field in {@code target}.
	 * @param index		An array index. It is guaranteed by the caller
	 *			that a negative value is not specified.
	 * @throws NullPointerException
	 *	{@code name} is null.
	 * @throws IllegalArgumentException
	 *	{@code name} is not a field name.
	 * @throws IllegalArgumentException
	 *	{@code name} is not an array field name.
	 * @throws IllegalArgumentException
	 *	Data type of the field specified by {@code name} is invalid.
	 * @throws ArrayIndexOutOfBoundsException
	 *	{@code index} is greater than or equal to its length.
	 * @see	IpcStruct#set(String, int, IpcDataUnit)
	 */
	@Override
	final void setStruct(IpcStruct target, String name, int index)
	{
		target.set(name, index, _value);
	}
}
