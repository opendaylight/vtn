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
import org.opendaylight.vtn.core.util.HostAddress;

/**
 * <p>
 *   The {@code ChannelAddress} represents an immutable IPC channel address.
 *   IPC channel address is used by IPC client to determine IPC server to
 *   connect.
 * </p>
 * <p>
 *   IPC channel address consists of IPC channel name and host address.
 *   IPC channel name is a string which determines type of services provided
 *   by IPC server. Host address determines the host on which IPC server is
 *   running.
 * </p>
 * <p>
 *   This class never validates the specified IPC channel name.
 *   An {@code IllegalArgumentException} will be thrown when an IPC channel
 *   address with an invalid IPC channel name is actually used.
 * </p>
 *
 * @since	C10
 * @see		AltConnection
 * @see		ConnectionPool
 */
public final class ChannelAddress
{
	/**
	 * Separator of IPC channel name and host address.
	 */
	private final static char ADDRESS_SEPARATOR = '@';

	/**
	 * IPC channel name.
	 */
	private final String  _channel;

	/**
	 * Host address.
	 */
	private final HostAddress  _host;

	/**
	 * Cache for hash code.
	 */
	private int _hash;

	/**
	 * <p>
	 *   Construct an IPC channel address which specifies IPC server
	 *   on the local host.
	 * </p>
	 * <p>
	 *   The local host is always chosen as the host address.
	 * </p>
	 *
	 * @param channel	IPC channel name. If null or an empty string
	 *			is specified, current IPC channel name of
	 *			{@link DefaultConnection} is used.
	 */
	public ChannelAddress(String channel)
	{
		this(channel, HostAddress.getLocalAddress());
	}

	/**
	 * <p>
	 *   Construct an IPC channel address which specifies IPC server
	 *   on the remote host.
	 * </p>
	 *
	 * @param channel	IPC channel name. If null or an empty string
	 *			is specified, current IPC channel name of
	 *			{@link DefaultConnection} is used.
	 * @param iaddr		IP address of the remote host.
	 * @throws NullPointerException
	 *	{@code iaddr} is null.
	 */
	public ChannelAddress(String channel, InetAddress iaddr)
	{
		this(channel, HostAddress.getHostAddress(iaddr));
	}

	/**
	 * <p>
	 *   Construct an IPC channel address which specifies IPC server
	 *   on the specified host.
	 * </p>
	 *
	 * @param channel	IPC channel name. If null or an empty string
	 *			is specified, current IPC channel name of
	 *			{@link DefaultConnection} is used.
	 * @param host		Host address which specifies the host.
	 * @throws NullPointerException
	 *	{@code host} is null.
	 */
	public ChannelAddress(String channel, HostAddress host)
	{
		if (host == null) {
			throw new NullPointerException("host is null.");
		}
		_channel = channel;
		_host = host;
	}

	/**
	 * Return IPC channel name of this address.
	 *
	 * @return	IPC channel name, or null if not set.
	 */
	public String getChannelName()
	{
		return _channel;
	}

	/**
	 * Return the host address of this address.
	 *
	 * @return	The host address instance of this address.
	 */
	public HostAddress getHostAddress()
	{
		return _host;
	}

	/**
	 * Return a string representation of this address.
	 *
	 * @return	A string representation of this IPC channel address.
	 */
	@Override
	public String toString()
	{
		String text = (_channel == null)
			? new String(new char[]{ADDRESS_SEPARATOR})
			: _channel + ADDRESS_SEPARATOR;

		return text + _host.toString();
	}

	/**
	 * Determine whether the specified object is identical to this
	 * IPC channel address or not.
	 *
	 * @param obj	An object to be compared.
	 * @return	True is returned if the specified object is identical
	 *		to this IPC channel address.
	 *		Otherwise false is returned.
	 */
	@Override
	public boolean equals(Object obj)
	{
		if (obj instanceof ChannelAddress) {
			ChannelAddress chaddr = (ChannelAddress)obj;
			if (_channel == null) {
				if (chaddr._channel != null) {
					return false;
				}
			}
			else if (!_channel.equals(chaddr._channel)) {
				return false;
			}

			return _host.equals(chaddr._host);
		}

		return false;
	}

	/**
	 * Return hash code of this IPC channel address.
	 *
	 * @return	A hash code.
	 */
	@Override
	public int hashCode()
	{
		int hash = _hash;

		if (hash == 0) {
			if (_channel != null) {
				hash = _channel.hashCode();
			}

			hash ^= _host.hashCode();
			_hash = hash;
		}

		return hash;
	}
}
