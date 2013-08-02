/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import java.util.Arrays;
import java.util.Enumeration;
import java.util.Random;
import java.util.HashSet;
import java.net.InetAddress;
import java.net.Inet4Address;
import java.net.Inet6Address;

/**
 * <p>
 *   Unit test class for {@link HostAddress}.
 * </p>
 */
public class HostAddressTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link HostAddress}.
	 *
	 * @param name	The test name.
	 */
	public HostAddressTest(String name)
	{
		super(name);
	}

	/**
	 * Test case for local address.
	 */
	public void testLocal()
	{
		HostAddress haddr = HostAddress.getLocalAddress();
		assertNotNull(haddr);
		assertSame(HostAddress.Type.LOCAL, haddr.getType());
		assertNull(haddr.getInetAddress());
		assertNull(haddr.getAddress());
		assertEquals("LOCAL", haddr.toString());
		assertEquals(0, haddr.getScopeId());

		try {
			haddr.getNumericAddress();
			needException();
		}
		catch (Exception e) {
			checkException
				(e, IllegalStateException.class,
				 "Local address has not network address.");
		}

		// Ensure that getByName(String) accepts a string
		// created by toString().
		HostAddress ha = HostAddress.getByName(haddr.toString());
		assertEquals(haddr, ha);

		for (int i = 0; i < 100; i++) {
			assertSame(haddr, HostAddress.getLocalAddress());
		}
	}

	/**
	 * Test case for IPv4 address.
	 */
	public void testInet4()
	{
		String[] addresses = {
			"0.0.0.0",
			"0.0.0.1",
			"10.1.1.1",
			"123.200.33.255",
			"127.0.0.1",
			"192.168.0.254",
			"192.168.1.255",
			"255.255.255.255",
		};
		long[] numeric = {
			0L,
			1L,
			0xa010101L,
			0x7bc821ffL,
			0x7f000001L,
			0xc0a800feL,
			0xc0a801ffL,
			0xffffffffL,
		};

		for (int i = 0; i < addresses.length; i++) {
			InetAddress  iaddr = null;

			try {
				iaddr = InetAddress.getByName(addresses[i]);
			}
			catch (Exception e) {
				fail("Unable to create InetAddress: " + e);
			}

			HostAddress haddr = HostAddress.getHostAddress(iaddr);
			assertNotNull(haddr);
			assertSame(HostAddress.Type.IPV4, haddr.getType());
			assertEquals(iaddr, haddr.getInetAddress());
			assertEquals(0, haddr.getScopeId());
			assertTrue(Arrays.equals(iaddr.getAddress(),
						 haddr.getAddress()));

			String str = addresses[i] + "/INET4";
			assertEquals(addresses[i] + "/INET4",
				     haddr.toString());

			long[] naddr = haddr.getNumericAddress();
			assertEquals(1, naddr.length);
			assertEquals(numeric[i], naddr[0]);

			// Ensure that getByName(String) accepts a string
			// created by toString().
			HostAddress ha = HostAddress.getByName
				(haddr.toString());
			assertEquals(haddr, ha);

			// Construct IPv4 host address from raw address.
			byte[] raw = iaddr.getAddress();
			for (int scope = -10; scope <= 10; scope++) {
				ha = HostAddress.getHostAddress(raw, scope);
				assertEquals(haddr, ha);
				assertEquals(0, ha.getScopeId());
			}
		}

		try {
			HostAddress.getHostAddress(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class,
				       "addr is null.");
		}
	}

	/**
	 * Test case for IPv6 address.
	 */
	public void testInet6()
	{
		// There is no way to test IPv6 address with scoped interface
		// name because NetworkInterface does not provide interface
		// to obtain interface index on JDK 1.6.
		String[] addresses = {
			"::",
			"::1",
			"100::abcd:ef99",
			"1:2:3:4:dead::beef",
			"aaaa:bbbb:cccc:dddd::",
			"2001:3000:aabb:ccdd::1234",
			"e000:f000:1111:2222:3333:4444:5555:6666",
			"ff02::1%3",
			"ff02::3%5",
			"ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff",
		};
		long[][] numeric = {
			new long[]{0L, 0L, 0L, 0L},
			new long[]{0L, 0L, 0L, 1L},
			new long[]{0x1000000L, 0L, 0L, 0xabcdef99L},
			new long[]{0x00010002L, 0x00030004L, 0xdead0000L,
				   0xbeefL},
			new long[]{0xaaaabbbbL, 0xccccddddL, 0L, 0L},
			new long[]{0x20013000L, 0xaabbccddL, 0L, 0x1234L},
			new long[]{0xe000f000L, 0x11112222L, 0x33334444L,
				   0x55556666L},
			new long[]{0xff020000L, 0L, 0L, 1L},
			new long[]{0xff020000L, 0L, 0L, 3L},
			new long[]{0xffffffffL, 0xffffffffL, 0xffffffffL,
				   0xffffffffL},
		};
		int[] scopeId = {
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			3,
			5,
			0,
		};

		for (int i = 0; i < addresses.length; i++) {
			InetAddress  iaddr = null;

			try {
				iaddr = InetAddress.getByName(addresses[i]);
			}
			catch (Exception e) {
				fail("Unable to create InetAddress: " + e);
			}

			HostAddress haddr = HostAddress.getHostAddress(iaddr);
			assertNotNull(haddr);
			assertSame(HostAddress.Type.IPV6, haddr.getType());
			assertEquals(iaddr, haddr.getInetAddress());
			assertEquals(scopeId[i], haddr.getScopeId());
			assertTrue(Arrays.equals(iaddr.getAddress(),
						 haddr.getAddress()));

			String saddr = haddr.toString();
			assertEquals(addresses[i] + "/INET6", saddr);

			// toString() should cache the result.
			assertSame(saddr, haddr.toString());

			long[] naddr = haddr.getNumericAddress();
			assertEquals(4, naddr.length);

			long[] reqaddr = numeric[i];
			for (int j = 0; j < naddr.length; j++) {
				assertEquals(reqaddr[j], naddr[j]);
			}

			// Ensure that getByName(String) accepts a string
			// created by toString().
			HostAddress ha = HostAddress.getByName
				(haddr.toString());
			assertEquals(haddr, ha);

			// Construct IPv6 host address from raw address.
			byte[] raw = iaddr.getAddress();
			ha = HostAddress.getHostAddress(raw, scopeId[i]);
			assertEquals(haddr, ha);
			assertEquals(scopeId[i], ha.getScopeId());

			// Zero or a negative scope ID must be ignored.
			for (int scope = -20; scope <= 0; scope++) {
				ha = HostAddress.getHostAddress(raw, scope);
				assertEquals(0, ha.getScopeId());
				assertTrue(Arrays.equals(raw,
							 ha.getAddress()));
			}
		}
	}

	/**
	 * Test case for {@link HostAddress#getHostAddress(byte[], int)}.
	 */
	public void testGetHostAddress()
	{
		// null address.
		byte[] raw = null;

		try {
			HostAddress.getHostAddress(raw, 0);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class, null);
		}

		// An address of illegal length.
		Class<?> cls = IllegalArgumentException.class;
		for (int len = 0; len < 100; len++) {
			if (len == 4 || len == 16) {
				continue;
			}

			raw = new byte[len];

			try {
				HostAddress.getHostAddress(raw, 0);
				needException();
			}
			catch (Exception e) {
				checkException(e, cls,
					       "Illegal length: " + len);
			}
		}
	}

	/**
	 * Test case for {@link HostAddress#getByName(String)}.
	 */
	public void testGetByName()
	{
		// Local address test.
		String[] strings = {
			null,
			"",
			"local",
			"Local",
			"LOCAL",
		};

		HostAddress local = HostAddress.getLocalAddress();
		for (String str: strings) {
			assertEquals(local, HostAddress.getByName(str));
		}

		// IPv4 address without suffix.
		strings = new String[] {
			"0.0.0.0",
			"127.0.0.1",
			"192.168.1.255",
			"255.255.255.255",
		};

		for (String str: strings) {
			InetAddress iaddr = null;
			try {
				iaddr = InetAddress.getByName(str);
			}
			catch (Exception e) {
				fail("Unable to create InetAddress: " + e);
			}

			assertTrue(iaddr instanceof Inet4Address);
			HostAddress haddr = HostAddress.getByName(str);
			assertEquals(iaddr, haddr.getInetAddress());
			assertEquals(HostAddress.Type.IPV4, haddr.getType());

			// Append suffix.
			str += "/INET4";
			HostAddress ha = HostAddress.getByName(str);
			assertEquals(haddr, ha);
			assertEquals(iaddr, ha.getInetAddress());
		}

		try {
			// "/INET4" will be converted into an IPv4 loopback
			// address.
			HostAddress haddr = HostAddress.getByName("/INET4");
			assertEquals(HostAddress.Type.IPV4, haddr.getType());

			InetAddress loop = InetAddress.getByName("127.0.0.1");
			InetAddress iaddr = haddr.getInetAddress();
			assertEquals(loop, iaddr);
			assertTrue(iaddr.isLoopbackAddress());
			assertEquals("127.0.0.1/INET4", haddr.toString());
		}
		catch (Exception e) {
			fail("Unexpected exception: " + e);
		}

		// IPv4 address with invalid suffix.
		strings = new String[] {
			"/INET4192.168.1.254",
			"192.168.1/INET4.254",
			"192.168.1.25/INET44",
		};

		Class<?> cls = IllegalArgumentException.class;
		for (String str: strings) {
			try {
				HostAddress.getByName(str);
				needException(str);
			}
			catch (Exception e) {
				checkException(e, cls,
					       "Invalid host address: " + str);
			}
		}

		// IPv6 address without suffix.
		strings = new String[] {
			"::",
			"::1",
			"100::abcd:ef99",
			"2001:3000:aabb:ccdd::1234",
			"e000:f000:1111:2222:3333:4444:5555:6666",
			"ff02::1%3",
			"ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff",
		};

		for (String str: strings) {
			InetAddress iaddr = null;
			try {
				iaddr = InetAddress.getByName(str);
			}
			catch (Exception e) {
				fail("Unable to create InetAddress: " + e);
			}

			assertTrue(iaddr instanceof Inet6Address);
			HostAddress haddr = HostAddress.getByName(str);
			assertEquals(iaddr, haddr.getInetAddress());
			assertEquals(HostAddress.Type.IPV6, haddr.getType());

			// Append suffix.
			str += "/INET6";
			HostAddress ha = HostAddress.getByName(str);
			assertEquals(haddr, ha);
			assertEquals(iaddr, ha.getInetAddress());
		}

		try {
			// "/INET6" will be converted into an IPv6 loopback
			// address.
			HostAddress haddr = HostAddress.getByName("/INET6");
			assertEquals(HostAddress.Type.IPV6, haddr.getType());

			InetAddress loop = InetAddress.getByName("::1");
			InetAddress iaddr = haddr.getInetAddress();
			assertEquals(loop, iaddr);
			assertTrue(iaddr.isLoopbackAddress());
			assertEquals("::1/INET6", haddr.toString());
		}
		catch (Exception e) {
			fail("Unexpected exception: " + e);
		}

		// IPv6 address with invalid suffix.
		strings = new String[] {
			"/INET62001:3000:aabb:ccdd::1234",
			"2001/INET6:3000:aabb:ccdd::1234",
			"2001:3000:aabb:/INET6ccdd::1234",
			"2001:3000:aabb:ccdd:/INET6:1234",
			"2001:3000:aabb:ccdd::123/INET64",
		};

		for (String str: strings) {
			try {
				HostAddress.getByName(str);
				needException(str);
			}
			catch (Exception e) {
				checkException(e, cls,
					       "Invalid host address: " + str);
			}
		}

		// Host address and suffix does not match.
		strings = new String[] {
			"2001:3000:aabb:ccdd::1234/INET4",
			"192.168.10.1/INET6",
		};
		for (String str: strings) {
			try {
				HostAddress.getByName(str);
				needException(str);
			}
			catch (Exception e) {
				checkException(e, cls,
					       "Invalid host address with " +
					       "type: " + str);
			}
		}
	}

	/**
	 * Test case for {@link HostAddress#compareTo(HostAddress)}.
	 */
	public void testCompareTo()
	{
		final int nelems = 1000;
		HostAddress[] array = createAddresses(nelems);

		// Sort HostAddress array.
		Arrays.sort(array);

		HostAddress.Type prevType = null;
		long[] prevNaddr = null;
		int prevScope = -1;
		for (int i = 0; i < nelems; i++) {
			HostAddress haddr = array[i];
			HostAddress.Type type = haddr.getType();

			// Local address must be the least.
			if (type == HostAddress.Type.LOCAL) {
				if (prevType == null) {
					prevType = type;
				}
				else {
					assertSame(type, prevType);
				}
				continue;
			}

			long[] naddr = haddr.getNumericAddress();

			// INET4 must be less than INET6, and be greater than
			// LOCAL.
			if (type == HostAddress.Type.IPV4) {
				assertEquals(1, naddr.length);

				if (prevType == null ||
				    prevType == HostAddress.Type.LOCAL) {
					prevType = type;
				}
				else {
					assertSame(prevType, type);
					assertEquals(1, prevNaddr.length);
					long n = naddr[0] & 0xffffffffL;
					long pn = prevNaddr[0] & 0xffffffffL;
					assertTrue(pn <= n);
				}

				prevNaddr = naddr;
				continue;
			}

			// INET6 must be greater than other types.
			assertSame(HostAddress.Type.IPV6, type);
			assertEquals(4, naddr.length);
			if (prevType == null ||
			    prevType == HostAddress.Type.LOCAL ||
			    prevType == HostAddress.Type.IPV4) {
				prevType = type;
			}
			else {
				assertSame(prevType, type);
				assertEquals(4, prevNaddr.length);
				int j;
				for (j = 0; j < naddr.length; j++) {
					long n = naddr[j];
					long pn = prevNaddr[j];

					if (n != pn) {
						assertTrue(pn < n);
						prevScope = -1;
						break;
					}
				}

				if (j == naddr.length) {
					Inet6Address in6 = (Inet6Address)
						haddr.getInetAddress();
					int scope = in6.getScopeId();
					assertTrue(prevScope <= scope);
					prevScope = scope;
				}
			}

			prevNaddr = naddr;
		}
	}

	/**
	 * Test case for {@link HostAddress#equals(Object)} and
	 * {@link HostAddress#hashCode()}.
	 */
	public void testEquals()
	{
		final int nelems = 1000;
		HostAddress[] array = createAddresses(nelems);
		HashSet<HostAddress> set = new HashSet<HostAddress>();

		// Sort HostAddress array.
		Arrays.sort(array);

		HostAddress local = null, prev = null;
		for (int i = 0; i < nelems; i++) {
			HostAddress haddr = array[i];
			HostAddress.Type type = haddr.getType();

			if (local == null &&
			    type == HostAddress.Type.LOCAL) {
				local = haddr;
			}

			if (set.contains(haddr)) {
				assertEquals(0, haddr.compareTo(prev));
				assertFalse(set.add(haddr));
			}
			else {
				assertTrue(set.add(haddr));
				if (prev != null) {
					assertTrue(haddr.compareTo(prev) != 0);
				}
			}

			prev = haddr;
		}

		HostAddress laddr = HostAddress.getLocalAddress();
		if (local != null) {
			assertSame(local, laddr);
			assertTrue(set.contains(laddr));
		}
	}

	/**
	 * Test case for {@link HostAddress#getAddress(InetAddress)}.
	 */
	public void testGetAddress()
	{
		Random rand = new Random();

		// IPv4 address.
		for (int loop = 0; loop < 0x10000; loop++) {
			int addr = rand.nextInt();
			byte[] raw = {
				(byte)((addr >>> 24) & 0xff),
				(byte)((addr >>> 16) & 0xff),
				(byte)((addr >>> 8) & 0xff),
				(byte)(addr & 0xff),
			};

			try {
				InetAddress iaddr =
					InetAddress.getByAddress(raw);
				String required =
					((int)raw[0] & 0xff) + "." +
					((int)raw[1] & 0xff) + "." +
					((int)raw[2] & 0xff) + "." +
					((int)raw[3] & 0xff);

				String text = HostAddress.getAddress(iaddr);
				assertEquals(required, text);
			}
			catch (Exception e) {
				fail("Unexpected exception: " + e);
			}
		}

		// IPv6 address.
		String[] addresses = {
			"::",
			"::1",
			"100::abcd:ef99",
			"1:2:3:4:dead::beef",
			"aaaa:bbbb:cccc:dddd::",
			"2001:3000:aabb:ccdd::1234",
			"3000:abcd:ef01:1122::5:3",
			"e000:f000:1111:2222:3333:4444:5555:6666",
			"ff02::1",
			"ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff",
		};
		byte[][] numeric = {
			new byte[]{0, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0},
			new byte[]{0, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 1},
			new byte[]{0x01, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, (byte)0xab, (byte)0xcd,
				   (byte)0xef, (byte)0x99},
			new byte[]{0x00, 0x01, 0x00, 0x02,
				   0x00, 0x03, 0x00, 0x04,
				   (byte)0xde, (byte)0xad, 0x00, 0x00,
				   0x00, 0x00, (byte)0xbe, (byte)0xef},
			new byte[]{(byte)0xaa, (byte)0xaa,
				   (byte)0xbb, (byte)0xbb,
				   (byte)0xcc, (byte)0xcc,
				   (byte)0xdd, (byte)0xdd,
				   0, 0, 0, 0, 0, 0, 0, 0},
			new byte[]{0x20, 0x01, 0x30, 0x00,
				   (byte)0xaa, (byte)0xbb,
				   (byte)0xcc, (byte)0xdd,
				   0, 0, 0, 0, 0, 0, 0x12, 0x34},
			new byte[]{0x30, 0x00, (byte)0xab, (byte)0xcd,
				   (byte)0xef, 0x01, 0x11, 0x22,
				   0, 0, 0, 0, 0x00, 0x05, 0x00, 0x03},
			new byte[]{(byte)0xe0, 0x00, (byte)0xf0, 0x00,
				   0x11, 0x11, 0x22, 0x22,
				   0x33, 0x33, 0x44, 0x44,
				   0x55, 0x55, 0x66, 0x66},
			new byte[]{(byte)0xff, 0x02, 0x00, 0x00,
				   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
			new byte[]{-1, -1, -1, -1, -1, -1, -1, -1,
				   -1, -1, -1, -1, -1, -1, -1, -1},
		};

		for (int i = 0; i < numeric.length; i++) {
			try {
				InetAddress  iaddr =
					InetAddress.getByAddress(numeric[i]);
				String text = HostAddress.getAddress(iaddr);
				assertEquals(addresses[i], text);

				// Ensure that scope ID is ignored.
				int id = i + 1;
				text = text + "%" + id;
				iaddr = InetAddress.getByName(text);
				Inet6Address i6addr = (Inet6Address)iaddr;
				assertEquals(id, i6addr.getScopeId());
				text = HostAddress.getAddress(iaddr);
				assertEquals(addresses[i], text);
			}
			catch (Exception e) {
				fail("Unexpected exception: " + e);
			}
		}

		// Specify null.
		try {
			HostAddress.getAddress(null);
			needException();
		}
		catch (NullPointerException e) {}
	}

	/**
	 * Create {@link HostAddress} array which keeps random host address.
	 *
	 * @param nelems	The number of array elements.
	 * @return		Array of host address.
	 */
	private HostAddress[] createAddresses(int nelems)
	{
		Random  rand = new Random();
		HostAddress[] array = new HostAddress[nelems];
		InetAddress prevIaddr = null;
		int scope = 1;

		for (int i = 0; i < nelems; i++) {
			HostAddress haddr;

			if (prevIaddr != null) {
				int r = rand.nextInt(4);

				if (r == 0) {
					haddr = HostAddress.
						getHostAddress(prevIaddr);
					array[i] = haddr;
					continue;
				}
				if (r == 1 &&
				    prevIaddr instanceof Inet6Address) {
					String str = prevIaddr.getHostAddress()
						+ "%" + scope;
					scope++;
					InetAddress iaddr = null;
					try {
						iaddr = InetAddress.
							getByName(str);
					}
					catch (Exception e) {
						fail("Unable to create " +
						     "InetAddress: " + e);
					}
					haddr = HostAddress.
						getHostAddress(iaddr);
					array[i] = haddr;
					continue;
				}
			}

			scope = 1;

			int c = rand.nextInt(3);
			if (c == 0) {
				haddr = HostAddress.getLocalAddress();
			}
			else {
				HostAddress.Type type;
				int size;
				if (c == 1) {
					size = 4;
					type = HostAddress.Type.IPV4;
				}
				else {
					size = 16;
					type = HostAddress.Type.IPV6;
				}

				byte[] addr = new byte[size];
				rand.nextBytes(addr);

				InetAddress iaddr = null;
				try {
					iaddr = InetAddress.getByAddress(addr);
				}
				catch (Exception e) {
					fail("Unable to create InetAddress: " +
					     e);
				}

				haddr = HostAddress.getHostAddress(iaddr);
				assertSame(type, haddr.getType());
				prevIaddr = iaddr;
			}

			assertEquals(0, haddr.compareTo(haddr));
			array[i] = haddr;
		}

		return array;
	}
}
