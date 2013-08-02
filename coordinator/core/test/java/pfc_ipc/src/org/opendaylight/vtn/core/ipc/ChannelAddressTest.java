/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import java.util.Random;
import java.util.HashSet;
import java.util.Iterator;
import java.net.InetAddress;
import java.net.Inet6Address;
import org.opendaylight.vtn.core.util.HostAddress;

/**
 * <p>
 *   Unit test class for {@link ChannelAddress}.
 * </p>
 */
public class ChannelAddressTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link ChannelAddress}.
	 *
	 * @param name	The test name.
	 */
	public ChannelAddressTest(String name)
	{
		super(name);
	}

	/**
	 * Test case for {@link ChannelAddress#ChannelAddress(String)}.
	 */
	public void testCtorLocal()
	{
		String[] names = {
			null,
			"",
			"a",
			"ab",
			"foo",
			"bar",
			"test",
		};

		for (int idx = 0; idx < names.length; idx++) {
			String name = names[idx];

			ChannelAddress chaddr = new ChannelAddress(name);
			assertEquals(name, chaddr.getChannelName());
			assertEquals(HostAddress.getLocalAddress(),
				     chaddr.getHostAddress());

			String str = (name == null) ? "@LOCAL"
				: name + "@LOCAL";
			assertEquals(str, chaddr.toString());
		}
	}

	/**
	 * Test case for
	 * {@link ChannelAddress#ChannelAddress(String, InetAddress)}.
	 */
	public void testCtorInetAddress()
	{
		String[] names = {
			null,
			"",
			"a",
			"ab",
			"foo",
			"bar",
			"test",
		};
		String[] addrs = {
			"127.0.0.1",
			"192.168.10.1",
			"100.200.254.1",
			"10.20.30.40",
			"::1",
			"fe80::123:abcd:ef01:9876%2",
			"abcd:ef01:1234:5678::1",
			"2001:200:dff:fff1:230:4567:ff12:789a",
		};

		for (int i = 0; i < names.length; i++) {
			for (int j = 0; j < addrs.length; j++) {
				String name = names[i];
				String addr = addrs[j];
				InetAddress iaddr = null;
				try {
					iaddr = InetAddress.getByName(addr);
				}
				catch (Exception e) {
					fail("Unable to create IP address: " +
					     addr + ": " + e);
				}

				ChannelAddress chaddr =
					new ChannelAddress(name, iaddr);
				HostAddress haddr =
					HostAddress.getHostAddress(iaddr);
				assertEquals(name, chaddr.getChannelName());
				assertEquals(haddr, chaddr.getHostAddress());

				String shaddr = addr;
				if (iaddr instanceof Inet6Address) {
					shaddr += "/INET6";
				}
				else {
					shaddr += "/INET4";
				}
				String sname = (name == null)
					? ""
					: name;
				assertEquals(sname + "@" + shaddr,
					     chaddr.toString());
			}
		}

		try {
			new ChannelAddress("name", (InetAddress)null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "addr is null.");
		}
	}

	/**
	 * Test case for
	 * {@link ChannelAddress#ChannelAddress(String, HostAddress)}.
	 */
	public void testCtorHostAddress()
	{
		String[] names = {
			null,
			"",
			"a",
			"ab",
			"foo",
			"bar",
			"test",
		};
		String[] addrs = {
			"127.0.0.1",
			"192.168.10.1",
			"100.200.254.1",
			"10.20.30.40",
			"::1",
			"fe80::123:abcd:ef01:9876%2",
			"abcd:ef01:1234:5678::1",
			"2001:200:dff:fff1:230:4567:ff12:789a",
		};

		for (int i = 0; i < names.length; i++) {
			for (int j = 0; j < addrs.length; j++) {
				String name = names[i];
				String addr = addrs[j];
				InetAddress iaddr = null;
				try {
					iaddr = InetAddress.getByName(addr);
				}
				catch (Exception e) {
					fail("Unable to create IP address: " +
					     addr + ": " + e);
				}

				HostAddress haddr =
					HostAddress.getHostAddress(iaddr);
				ChannelAddress chaddr =
					new ChannelAddress(name, haddr);
				assertEquals(name, chaddr.getChannelName());
				assertEquals(haddr, chaddr.getHostAddress());

				String shaddr = addr;
				if (iaddr instanceof Inet6Address) {
					shaddr += "/INET6";
				}
				else {
					shaddr += "/INET4";
				}
				String sname = (name == null)
					? ""
					: name;
				assertEquals(sname + "@" + shaddr,
					     chaddr.toString());
			}
		}

		try {
			new ChannelAddress("name", (HostAddress)null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "host is null.");
		}
	}

	/**
	 * Test case for {@link ChannelAddress#equals(Object)} and
	 * {@link ChannelAddress#hashCode()}.
	 */
	public void testEquals()
	{
		String[] names = {
			null,
			"",
			"a",
			"ab",
			"abc",
			"abcd",
			"abcde",
			"abcdef",
		};
		String[] addrs = {
			null,
			"0.0.0.1",
			"127.0.0.1",
			"192.168.10.1",
			"100.200.254.1",
			"10.20.30.40",
			"::1",
			"fe80::123:abcd:ef01:9876%1",
			"fe80::123:abcd:ef01:9876%2",
			"fe80::123:abcd:ef01:9876%3",
			"abcd:ef01:1234:5678::1",
			"2001:200:dff:fff1:230:4567:ff12:789a",
		};

		Random rand = new Random();
		HashSet<ChannelAddress> set = new HashSet<ChannelAddress>();

		for (int i = 0; i < names.length; i++) {
			for (int j = 0; j < addrs.length; j++) {
				testEqualsImpl(set, names[i], addrs[j]);
			}
		}
	}

	/**
	 * Internal method of {@link #testEquals()}.
	 *
	 * @param set	A hash set which keeps channel addresses.
	 * @param name	An IPC channel name.
	 * @param addr	A string which represents host address.
	 */
	private void testEqualsImpl(HashSet<ChannelAddress> set,
				    String name, String addr)
	{
		ChannelAddress chaddr, chaddr2;

		if (addr == null) {
			chaddr = new ChannelAddress(name);
			chaddr2 = new ChannelAddress(name);
		}
		else {
			InetAddress iaddr = null;
			try {
				iaddr = InetAddress.getByName(addr);
			}
			catch (Exception e) {
				fail("Unable to create IP address: " + addr +
				     ": " + e);
			}
			chaddr = new ChannelAddress(name, iaddr);
			chaddr2 = new ChannelAddress(name, iaddr);
		}

		assertEquals(chaddr, chaddr2);
		assertEquals(chaddr2, chaddr);
		assertEquals(chaddr.hashCode(), chaddr2.hashCode());
		assertFalse(chaddr.equals(null));
		assertFalse(chaddr.equals(new Object()));

		for (Iterator<ChannelAddress> it = set.iterator();
		     it.hasNext();) {
			ChannelAddress ca = it.next();

			assertFalse(chaddr.equals(ca));
			assertFalse(ca.equals(chaddr));
		}

		assertTrue(set.add(chaddr));
		assertFalse(set.add(chaddr));
	}
}
