/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import java.net.InetAddress;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.UnknownHostException;

/**
 * <p>
 *   The {@code HostAddress} class represents an abstract host address.
 *   {@code HostAddress} has three types.
 * </p>
 * <dl>
 *   <dt>Local Address</dt>
 *   <dd>
 *     <p>
 *       The local host.
 *     </p>
 *   </dd>
 *   <dt>IPv4 Address</dt>
 *   <dd>
 *     <p>
 *       The host address determine by IPv4 address.
 *     </p>
 *   </dd>
 *   <dt>IPv6 Address</dt>
 *   <dd>
 *     <p>
 *       The host address determine by IPv6 address.
 *     </p>
 *   </dd>
 * </dl>
 *
 * @since	C10
 */
public abstract class HostAddress implements Comparable<HostAddress>
{
	/**
	 * The {@code HostAddress.Type} enum defines constants which
	 * represents the type of {@link HostAddress}.
	 *
	 * @see		HostAddress
	 * @since	C10
	 */
	public static enum Type
	{
		/**
		 * Host address type assigned to local address.
		 */
		LOCAL,

		/**
		 * Host address type assigned to IPv4 address.
		 */
		IPV4,

		/**
		 * Host address type assigned to IPv6 address.
		 */
		IPV6,
	}

	/**
	 * Load native library.
	 */
	static {
		PfcUtil.load();
	}

	/**
	 * The first character of address type suffix.
	 */
	private final static char  SUFFIX_CHAR = '/';

	/**
	 * Return a {@code HostAddress} instance which represents a local
	 * address.
	 *
	 * @return	A local host address instance.
	 */
	public final static HostAddress getLocalAddress()
	{
		return Local._theInstance;
	}

	/**
	 * <p>
	 *   Return a {@code HostAddress} instance associated with the
	 *   specified IP address.
	 * </p>
	 * <ul>
	 *   <li>
	 *     If {@code addr} is {@code Inet4Address} instance, an IPv4
	 *     address is returned.
	 *   </li>
	 *   <li>
	 *     If {@code addr} is {@code Inet6Address}, an IPv6 address is
	 *     returned. The scoped network interface in it is converted into
	 *     scope ID.
	 *   </li>
	 * </ul>
	 *
	 * @param addr	An IP address object.
	 * @return	An IPv4 or IPv6 host address instance.
	 * @throws NullPointerException
	 *	{@code addr} is null.
	 * @throws IllegalArgumentException
	 *	Unexpected address is specified.
	 */
	public final static HostAddress getHostAddress(InetAddress addr)
	{
		if (addr instanceof Inet4Address) {
			return new Inet4(addr);
		}
		if (addr instanceof Inet6Address) {
			return new Inet6(addr);
		}

		if (addr == null) {
			throw new NullPointerException("addr is null.");
		}

		// This should never happen.
		throw new IllegalArgumentException("Unexpected address: " +
						   addr);
	}

	/**
	 * <p>
	 *   Return a {@code HostAddress} instance associated with the
	 *   specified raw IP address and scoped network interface ID.
	 * </p>
	 * <p>
	 *   {@code raw} must be a {@code byte} array which contains a raw
	 *   IP address in network byte order. Type of IP address is determined
	 *   by its length.
	 * </p>
	 * <p>
	 *   {@code scope} must be a scoped network interface ID for IPv6
	 *   address. Zero or a negative integer indicates that it is not
	 *   defined. {@code scope} is simply ignored if {@code raw} contains
	 *   a raw IPv4 address.
	 * </p>
	 *
	 * @param raw		A {@code byte} array which contains a raw IP
	 *			address in network byte order.
	 * @param scope		Scoped network interface ID.
	 * @return		An IPv4 or IPv6 host address instance.
	 * @throws NullPointerException
	 *	{@code raw} is null.
	 * @throws IllegalArgumentException
	 *	The specified address is of illegal length.
	 */
	public final static HostAddress getHostAddress(byte[] raw, int scope)
	{
		// InetAddress.getByAddress() returns Inet4Address if the byte
		// array contains an IPv4 mapped IPv6 address. So we must use
		// Inet6Address.getByAddress() explicitly if the specified
		// array contains an IPv6 address.
		int len = raw.length;

		try {
			InetAddress	addr;

			if (len == Inet6.ADDRLEN) {
				if (scope == 0) {
					scope = -1;
				}

				addr = Inet6Address.
					getByAddress(null, raw, scope);

				return new Inet6(addr);
			}
			else {
				addr = InetAddress.getByAddress(raw);

				return new Inet4(addr);
			}
		}
		catch (Exception e) {
			throw new IllegalArgumentException
				("Illegal length: " + raw.length);
		}
	}

	/**
	 * <p>
	 *   Return a {@code HostAddress} instance represented by the
	 *   specified string.
	 *   This method always accepts a string created by
	 *   {@link #toString()}.
	 * </p>
	 * <p>
	 *   This method returns a {@code HostAddress} instance which
	 *   represents a local address if {@code str} matches the one of
	 *   the followings.
	 * </p>
	 * <ul>
	 *   <li>{@code null}</li>
	 *   <li>An empty string</li>
	 *   <li>{@code "LOCAL"} (case-insensitive)</li>
	 * </ul>
	 * <p>
	 *   Otherwise {@code str} is considered as a string representation
	 *   of IP address or remote host name. Address type can be specified
	 *   by optional string suffix.
	 * </p>
	 * <ul>
	 *   <li>
	 *     If {@code str} ends with {@code "/INET4"}, this methods always
	 *     converts the given string into {@code Inet4Address}.
	 *     For example, {@code "www.foo.com/INET4"} is converted into
	 *     IPv4 address assigned to the host name {@code "www.foo.com"}.
	 *     Note that an IPv4 loopback address is returned if
	 *     {@code "/INET4"} is specified.
	 *   </li>
	 *   <li>
	 *     If {@code str} ends with {@code "/INET6"}, this methods always
	 *     converts the given string into {@code Inet6Address}.
	 *     For example, {@code "www.foo.com/INET6"} is converted into
	 *     IPv6 address assigned to the host name {@code "www.foo.com"}.
	 *     Note that an IPv6 loopback address is returned if
	 *     {@code "/INET6"} is specified.
	 *   </li>
	 * </ul>
	 *
	 * @param str	A string representation of host address.
	 * @return	A {@code HostAddress} instance created by {@code name}.
	 * @throws IllegalArgumentException
	 *	An invalid string is specified to {@code str}.
	 */
	public final static HostAddress getByName(String str)
	{
		int len;

		if (str == null || (len = str.length()) == 0 ||
		    str.equalsIgnoreCase(Local.ADDR)) {
			return getLocalAddress();
		}

		// Eliminate address type suffix.
		Class<?> cls = null;
		String orgstr = str;
		int idx = str.lastIndexOf(SUFFIX_CHAR);
		if (idx >= 0) {
			if (str.indexOf(Inet4.SUFFIX, idx) == idx &&
			    idx + Inet4.SUFFIX.length() == len) {
				str = str.substring(0, idx);
				cls = Inet4Address.class;
			}
			else if (str.indexOf(Inet6.SUFFIX, idx) == idx &&
				 idx + Inet6.SUFFIX.length() == len) {
				str = str.substring(0, idx);
				cls = Inet6Address.class;
				if (str.length() == 0) {
					// Specify IPv6 loopback address.
					str = "::1";
				}
			}
		}

		if (cls == null) {
			InetAddress iaddr;
			try {
				iaddr = InetAddress.getByName(str);
			}
			catch (UnknownHostException e) {
				throw new IllegalArgumentException
					("Invalid host address: " + str, e);
			}

			return getHostAddress(iaddr);
		}

		// Get all IP addresses associated with the given name,
		// and choose one address that matches the specified address
		// type.
		InetAddress[] iaddrs;
		try {
			iaddrs = InetAddress.getAllByName(str);
		}
		catch (UnknownHostException e) {
			throw new IllegalArgumentException
				("Invalid host address: " + str, e);
		}

		for (InetAddress iaddr: iaddrs) {
			if (cls.isInstance(iaddr)) {
				return getHostAddress(iaddr);
			}
		}

		throw new IllegalArgumentException
			("Invalid host address with type: " + orgstr);
	}

	/**
	 * <p>
	 *   Return a string representation of the specified IP address.
	 * </p>
	 * <ul>
	 *   <li>
	 *     If {@code addr} is an {@code Inet4Address} object, this method
	 *     returns the IP address string in text form
	 *     {@code "XXX.XXX.XXX.XXX"}.
	 *   </li>
	 *   <li>
	 *     If {@code addr} is an {@code Inet6Address} object, this method
	 *     returns the IP address string in text form.
	 *     Unlike {@code Inet6Address}, this method always compresses
	 *     the IP address string as possible. Note that the scoped network
	 *     interface in {@code addr} is always ignored.
	 *   </li>
	 * </ul>
	 *
	 * @param addr	An IP address object.
	 * @return	A string representation of {@code addr}.
	 * @throws NullPointerException
	 *	{@code addr} is null.
	 */
	public final static String getAddress(InetAddress addr)
	{
		if (!(addr instanceof Inet6Address)) {
			return addr.getHostAddress();
		}

		return toInet6TextAddress(addr.getAddress());
	}

	/**
	 * Convert raw IPv6 address into a string representation of IPv6
	 * address.
	 *
	 * @param addr	A byte array which represents IPv6 address
	 *		in network byte order.
	 * @return	A string representation of IPv6 address.
	 * @throws IllegalStateException
	 *	Unexpected address is specified to {@code addr}.
	 */
	private static native String toInet6TextAddress(byte[] addr);

	/**
	 * Private constructor.
	 */
	private HostAddress() {}

	/**
	 * Return the type of this host address.
	 *
	 * @return	{@link Type#LOCAL}, {@link Type#IPV4}, or
	 *		{@link Type#IPV6} is returned if the type of this
	 *		host address is local address, IPv4 address, or
	 *		IPv6 address respectively.
	 */
	public abstract Type getType();

	/**
	 * Return an IP address of this host address.
	 *
	 * @return	A network address of this host address.
	 *		null is returned if this host address is local address.
	 */
	public abstract InetAddress getInetAddress();

	/**
	 * Return a {@code byte} array which contains raw IP address of this
	 * host address.
	 *
	 * @return	A {@code byte} array which contains raw IP address
	 *		of this host address, or {@code} for local address.
	 */
	public final byte[] getAddress()
	{
		InetAddress iaddr = getInetAddress();

		return (iaddr == null) ? null : iaddr.getAddress();
	}

	/**
	 * <p>
	 *   Return the scoped network interface ID.
	 * </p>
	 * <p>
	 *   This method always returns zero if this instance is not
	 *   {@link Type#IPV6} address.
	 * </p>
	 *
	 * @return	The scoped network interface ID or zero.
	 */
	public int getScopeId()
	{
		return 0;
	}

	/**
	 * <p>
	 *   Compare this host address with the specified host address.
	 * </p>
	 * <ul>
	 *   <li>
	 *     <p>
	 *       Local address is the least.
	 *     </p>
	 *   </li>
	 *   <li>
	 *     <p>
	 *       IPv4 address is greater than local address, but less than
	 *       IPv6 address.
	 *     </p>
	 *     <p>
	 *       if both this host address and the specified address are IPv4
	 *       address, this method compares numerical network address.
	 *     </p>
	 *   </li>
	 *   <li>
	 *     <p>
	 *       IPv6 address is the greatest.
	 *     </p>
	 *     <p>
	 *       if both this host address and the specified address are IPv6
	 *       address, this method compares numerical network address.
	 *       If the scope ID is specified, it is compared in numeric order.
	 *     </p>
	 *   </li>
	 * </li>
	 *
	 * @param haddr		A host address to be compared.
	 * @return		A negative integer, zero, or a positive integer
	 *			is returned if this host address is less
	 *			than, equal to, or greater than the specified
	 *			host address respectively.
	 */
	@Override
	public final int compareTo(HostAddress haddr)
	{
		Type type = getType();
		int t = type.ordinal();
		int ht = haddr.getType().ordinal();
		int diff = t - ht;

		if (diff != 0 || type == Type.LOCAL) {
			return diff;
		}

		long[] bytes = getNumericAddress();
		long[] hbytes = haddr.getNumericAddress();

		for (int i = 0; i < bytes.length; i++) {
			long d = bytes[i] - hbytes[i];

			if (d != 0) {
				return (d < 0) ? -1 : 1;
			}
		}

		return compareToImpl(haddr);
	}

	/**
	 * Class-specific work of {@link #compareTo(HostAddress)}.
	 *
	 * @param haddr		A host address to be compared.
	 * @return		A negative integer, zero, or a positive integer
	 *			is returned if this host address is less
	 *			than, equal to, or greater than the specified
	 *			host address respectively.
	 */
	int compareToImpl(HostAddress haddr)
	{
		return 0;
	}

	/**
	 * Return numeric network address of this host address.
	 * The result is in host byte order.
	 *
	 * @return	An array of long which contains numeric network
	 *		address.
	 * @throws IllegalStateException
	 *	This host address is local address.
	 */
	abstract long[] getNumericAddress();

	/**
	 * This class represents a local address.
	 */
	private final static class Local extends HostAddress
	{
		/**
		 * A single local address instance.
		 */
		private final static Local _theInstance = new Local();

		/**
		 * A string which represents local address.
		 */
		private final static String ADDR = "LOCAL";

		/**
		 * Return the type of this host address.
		 *
		 * @return	{@link Type#LOCAL} is returned.
		 */
		@Override
		public Type getType()
		{
			return Type.LOCAL;
		}

		/**
		 * Return an IP address of this host address.
		 *
		 * @return	null is always returned.
		 */
		@Override
		public InetAddress getInetAddress()
		{
			return null;
		}

		/**
		 * Return a string representation of this local address.
		 *
		 * @return	"LOCAL" is always returned.
		 */
		@Override
		public String toString()
		{
			return ADDR;
		}

		/**
		 * This method must not be called.
		 *
		 * @throws IllegalStateException	Always thrown.
		 */
		@Override
		long[] getNumericAddress()
		{
			throw new IllegalStateException
				("Local address has not network address.");
		}
	}

	/**
	 * The base class of IPv4 and IPv6 address.
	 */
	private static abstract class Inet extends HostAddress
	{
		/**
		 * Internet address.
		 */
		final InetAddress  _address;

		/**
		 * Cache for {@link #getNumericAddress()}.
		 */
		long[]  _numericAddress;

		/**
		 * Construct an {@code Inet} instance.
		 *
		 * @param addr	IP address.
		 */
		private Inet(InetAddress addr)
		{
			_address = addr;
		}

		/**
		 * Return an IP address of this host address.
		 *
		 * @return	An IP address of this host address.
		 */
		@Override
		public final InetAddress getInetAddress()
		{
			return _address;
		}
	}

	/**
	 * This class represents an IPv6 address.
	 */
	private final static class Inet4 extends Inet
	{
		/**
		 * Suffix of IPv4 address.
		 */
		private final static String  SUFFIX = SUFFIX_CHAR + "INET4";

		/**
		 * Construct an IPv4 address.
		 *
		 * @param addr	IPv4 address.
		 */
		private Inet4(InetAddress addr)
		{
			super(addr);
			assert addr instanceof Inet4Address
				: "Invalid IP address: " + addr;
		}

		/**
		 * Return the type of this host address.
		 *
		 * @return	{@link Type#IPV4} is returned.
		 */
		@Override
		public Type getType()
		{
			return Type.IPV4;
		}

		/**
		 * Return a string representation of this IPv4 address.
		 *
		 * @return	A string representation of IPv4 address.
		 */
		@Override
		public String toString()
		{
			return _address.getHostAddress() + SUFFIX;
		}

		/**
		 * Determine whether the specified object is identical to
		 * this host address or not.
		 *
		 * @param obj	An object to be compared.
		 * @return	True is returned if the specified object is
		 *		identical to this host address.
		 *		Otherwise false is returned.
		 */
		@Override
		public boolean equals(Object obj)
		{
			if (obj instanceof Inet) {
				Inet h = (Inet)obj;

				return _address.equals(h._address);
			}

			return false;
		}

		/**
		 * Return a has code of this instance.
		 *
		 * @return	A hash code.
		 */
		@Override
		public int hashCode()
		{
			return _address.hashCode();
		}

		/**
		 * Return numeric IPv4 address of this host address.
		 *
		 * @return	An array of long which contains numeric
		 *		network address.
		 */
		@Override
		long[] getNumericAddress()
		{
			if (_numericAddress == null) {
				byte[] bytes = _address.getAddress();
				long l = ((bytes[0] << 24) & 0xff000000L) |
					((bytes[1] << 16) & 0x00ff0000L) |
					((bytes[2] << 8) & 0x0000ff00L) |
					(bytes[3] & 0x000000ffL);
				_numericAddress = new long[]{l};
			}

			return _numericAddress;
		}
	}

	/**
	 * This class represents an IPv6 address.
	 */
	private final static class Inet6 extends Inet
	{
		/**
		 * The number of bytes in an IPv4 address.
		 */
		private final static int  ADDRLEN = 16;

		/**
		 * Suffix of IPv6 address.
		 */
		private final static String  SUFFIX = SUFFIX_CHAR + "INET6";

		/**
		 * Cache for {@link #toString()}.
		 */
		private String  _text;

		/**
		 * Construct an IPv6 address.
		 *
		 * @param addr	IPv6 address.
		 */
		private Inet6(InetAddress addr)
		{
			super(addr);
			assert addr instanceof Inet6Address
				: "Invalid IP address: " + addr;
		}

		/**
		 * Return the type of this host address.
		 *
		 * @return	{@link Type#IPV6} is returned.
		 */
		@Override
		public Type getType()
		{
			return Type.IPV6;
		}

		/**
		 * Return the scoped network interface ID in the IPv6 address.
		 *
		 * @return	The scoped network interface ID.
		 *		Zero is returned if no scoped ID is set in
		 *		the IPv6 address.
		 */
		@Override
		public int getScopeId()
		{
			return ((Inet6Address)_address).getScopeId();
		}

		/**
		 * Return a string representation of this IPv6 address.
		 *
		 * @return	A string representation of IPv6 address.
		 */
		@Override
		public String toString()
		{
			String text = _text;

			if (text == null) {
				// We don't want to use
				// Inet6Address.getHostAddress() because
				// it may not shorten IPv6 address.
				Inet6Address iaddr = (Inet6Address)_address;
				text = HostAddress.getAddress(iaddr);

				int scope = iaddr.getScopeId();
				if (scope != 0) {
					text = text + "%" + scope;
				}

				text = text + SUFFIX;
				_text = text;
			}

			return text;
		}

		/**
		 * Determine whether the specified object is identical to
		 * this host address or not.
		 *
		 * @param obj	An object to be compared.
		 * @return	True is returned if the specified object is
		 *		identical to this host address.
		 *		Otherwise false is returned.
		 */
		@Override
		public boolean equals(Object obj)
		{
			if (obj instanceof Inet6) {
				Inet6Address iaddr = (Inet6Address)_address;
				Inet6Address oiaddr =
					(Inet6Address)(((Inet)obj)._address);
				return (iaddr.equals(oiaddr) &&
					iaddr.getScopeId() ==
					oiaddr.getScopeId());
			}

			return false;
		}

		/**
		 * Return a has code of this instance.
		 *
		 * @return	A hash code.
		 */
		@Override
		public int hashCode()
		{
			return _address.hashCode() +
				((Inet6Address)_address).getScopeId();
		}

		/**
		 * Return numeric IPv6 address of this host address.
		 *
		 * @return	An array of long which contains numeric
		 *		network address.
		 */
		@Override
		long[] getNumericAddress()
		{
			long[] naddr = _numericAddress;
			if (naddr == null) {
				byte[] bytes = _address.getAddress();
				int bidx = 0;
				naddr = new long[4];
				for (int i = 0; i < naddr.length; i++) {
					long l = (bytes[bidx] << 24) &
						0xff000000L;
					bidx++;
					l |= (bytes[bidx] << 16) & 0x00ff0000L;
					bidx++;
					l |= (bytes[bidx] << 8) & 0x0000ff00L;
					bidx++;
					l |= bytes[bidx] & 0x000000ffL;
					bidx++;
					naddr[i] = l;
				}

				_numericAddress = naddr;
			}

			return naddr;
		}

		/**
		 * IPv6 specific work of {@link #compareTo(HostAddress)}.
		 * This compares scope ID.
		 *
		 * @param haddr		A host address to be compared.
		 * @return		A negative integer, zero, or a
		 *			positive integer is returned if this
		 *			host address is less than, equal to,
		 *			or greater than the specified host
		 *			address respectively.
		 */
		@Override
		int compareToImpl(HostAddress haddr)
		{
			// The specified address must be IPv6 address.
			Inet6Address iaddr = (Inet6Address)_address;
			Inet6 in6 = (Inet6)haddr;
			Inet6Address hiaddr = (Inet6Address)in6._address;

			int id = iaddr.getScopeId();
			int hid = hiaddr.getScopeId();
			if (id == hid) {
				return 0;
			}

			return (id < hid) ? -1 : 1;
		}
	}
}
