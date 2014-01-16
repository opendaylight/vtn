/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import java.util.Arrays;
import java.util.HashSet;
import java.util.Random;
import java.net.InetAddress;
import java.net.Inet4Address;
import java.net.Inet6Address;

/**
 * <p>
 *   Unit test class for {@link IpcInet6Address}.
 * </p>
 */
public class IpcInet6AddressTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link IpcInet6Address}.
	 *
	 * @param name	The test name.
	 */
	public IpcInet6AddressTest(String name)
	{
		super(name);
	}

	/**
	 * Test case for {@link IpcInetAddress#create(InetAddress)}.
	 */
	public void testCreateInetAddress()
	{
		Random rand = new Random();

		for (int loop = 0; loop < 0x10000; loop++) {
			byte[] raw = new byte[16];
			rand.nextBytes(raw);

			try {
				InetAddress iaddr =
					InetAddress.getByAddress(raw);
				IpcInetAddress obj =
					IpcInetAddress.create(iaddr);
				checkValue(obj, raw);

				IpcInetAddress objInet4 =
					IpcInet4Address.create(iaddr);
				checkValue(objInet4, raw);

				IpcInetAddress objInet6 =
					IpcInet6Address.create(iaddr);
				checkValue(objInet6, raw);
			}
			catch (Exception e) {
				fail("Unexpected exception: " + e);
			}
		}

		// Specify null.
		try {
			IpcInet6Address.create((InetAddress)null);
			needException();
		}
		catch (NullPointerException e) {}
	}

	/**
	 * Test case for {@link IpcInetAddress#create(byte[])}.
	 */
	public void testCreateByteArray()
	{
		Random rand = new Random();

		byte[] raw = new byte[16];
		for (int loop = 0; loop < 0x10000; loop++) {
			rand.nextBytes(raw);

			try {
				IpcInetAddress obj =
					IpcInetAddress.create(raw);
				checkValue(obj, raw);

				IpcInetAddress objInet4 =
					IpcInet4Address.create(raw);
				checkValue(objInet4, raw);

				IpcInetAddress objInet6 =
					IpcInet6Address.create(raw);
				checkValue(objInet6, raw);
			}
			catch (Exception e) {
				fail("Unexpected exception: " + e);
			}
		}

		// Ensure that IpcInet6Address is created even an IPv4 mapped
		// IPv6 address is specified.
		byte[] ipv4 = new byte[4];
		for (int i = 0; i < 10; i++) {
			raw[i] = 0;
		}
		raw[10] = (byte)0xff;
		raw[11] = (byte)0xff;

		for (int loop = 0; loop < 0x10000; loop++) {
			rand.nextBytes(ipv4);
			System.arraycopy(ipv4, 0, raw, 12, ipv4.length);

			try {
				InetAddress iaddr = InetAddress.
					getByAddress(raw);
				assertTrue(iaddr instanceof Inet4Address);

				IpcInetAddress obj =
					IpcInetAddress.create(raw);
				checkValue(obj, raw);

				IpcInetAddress objInet4 =
					IpcInet4Address.create(raw);
				checkValue(objInet4, raw);

				IpcInetAddress objInet6 =
					IpcInet6Address.create(raw);
				checkValue(objInet6, raw);
			}
			catch (Exception e) {
				fail("Unexpected exception: " + e);
			}
		}

		// Specify null.
		try {
			IpcInet6Address.create((byte[])null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class, null);
		}

		// Specify invalid array.
		for (int i = 0; i <= 100; i++) {
			if (i == 4 || i == 16) {
				continue;
			}

			raw = new byte[i];
			try {
				IpcInet6Address.create(raw);
				needException(i);
			}
			catch (Exception e) {
				assertEquals(IllegalArgumentException.class,
					     e.getClass());
			}
		}
	}

	/**
	 * Test case for {@link IpcInetAddress#toString()}.
	 */
	public void testToString()
	{
		String[] required = {
			"::",
			"::1",
			"::2",
			"ff02::1",
			"2001:abcd:ef12:10::32",
			"1111:2222:3333:4444:5555:6666:7777:8888",
			"aabb:ccdd::ef00",
			"1:2:3:4::",
			"dead::beef:badd:cafe",
			"ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff",
			"::ffff:192.168.0.1",
		};
		byte[][] addresses = {
			new byte[]{0, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 0},
			new byte[]{0, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 1},
			new byte[]{0, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 2},
			new byte[]{(byte)0xff, 0x02, 0, 0, 0, 0, 0, 0,
				   0, 0, 0, 0, 0, 0, 0, 1},
			new byte[]{0x20, 0x01, (byte)0xab, (byte)0xcd,
				   (byte)0xef, 0x12, 0x00, 0x10,
				   0, 0, 0, 0, 0, 0, 0, 0x32},
			new byte[]{0x11, 0x11, 0x22, 0x22,
				   0x33, 0x33, 0x44, 0x44,
				   0x55, 0x55, 0x66, 0x66,
				   0x77, 0x77, (byte)0x88, (byte)0x88},
			new byte[]{(byte)0xaa, (byte)0xbb,
				   (byte)0xcc, (byte)0xdd,
				   0, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, (byte)0xef, 0x00},
			new byte[]{0, 0x1, 0, 0x2, 0, 0x3, 0, 0x4,
				   0, 0, 0, 0, 0, 0, 0, 0},
			new byte[]{(byte)0xde, (byte)0xad, 0, 0, 0, 0, 0, 0,
				   0, 0, (byte)0xbe, (byte)0xef,
				   (byte)0xba, (byte)0xdd,
				   (byte)0xca, (byte)0xfe},
			new byte[]{-1, -1, -1, -1, -1, -1, -1, -1,
				   -1, -1, -1, -1, -1, -1, -1, -1},
			new byte[]{0, 0, 0, 0, 0, 0, 0, 0,
				   0, 0, (byte)0xff, (byte)0xff,
				   (byte)192, (byte)168, 0, 1},
		};

		for (int idx = 0; idx < addresses.length; idx++) {
			try {
				byte[] raw = addresses[idx];
				String req = required[idx];
				IpcInetAddress obj =
					IpcInetAddress.create(raw);
				assertEquals(req, obj.toString());

				IpcInetAddress objInet4 =
					IpcInet4Address.create(raw);
				assertEquals(req, obj.toString());

				IpcInetAddress objInet6 =
					IpcInet6Address.create(raw);
				assertEquals(req, obj.toString());

				// Ensure that the scoped ID is ignored.
				int scope = idx + 1;
				String text = req + "%" + scope;
				InetAddress iaddr = InetAddress.getByName(text);
				if (iaddr instanceof Inet4Address) {
					// IP
					iaddr = Inet6Address.getByAddress(null, raw,
									  scope);
				}
				int id = ((Inet6Address)iaddr).getScopeId();
				assertEquals(scope, id);

				obj = IpcInetAddress.create(iaddr);
				assertEquals(req, obj.toString());

				objInet4 = IpcInet4Address.create(raw);
				assertEquals(req, obj.toString());

				objInet6 = IpcInet6Address.create(raw);
				assertEquals(req, obj.toString());

			}
			catch (Exception e) {
				fail("Unexpected exception: " + e);
			}
		}
	}

	/**
	 * Test case for {@link IpcInetAddress#equals(Object)} and
	 * {@link IpcInetAddress#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<IpcInetAddress> set = new HashSet<IpcInetAddress>();
		HashSet<InetAddress> iset = new HashSet<InetAddress>();
		Random rand = new Random();

		IpcInetAddress obj4 = null;
		try {
			InetAddress addr4 =
				InetAddress.getByName("192.168.10.1");
			obj4 = IpcInetAddress.create(addr4);
		}
		catch (Exception e) {
			fail("Unexpected exception: " + e);
		}

		for (int loop = 0; loop < 100; loop++) {
			byte[] raw = new byte[16];
			rand.nextBytes(raw);

			try {
				IpcInetAddress obj =
					IpcInetAddress.create(raw);
				assertTrue(obj.equals(obj));
				assertFalse(obj.equals(null));
				assertFalse(obj.equals(new Object()));
				assertFalse(obj.equals(obj4));

				InetAddress iaddr = obj.getValue();
				if (iset.add(iaddr)) {
					assertTrue(set.add(obj));
					assertFalse(set.add(obj));
				}

				obj = IpcInetAddress.create(raw);
				assertTrue(obj.equals(obj));

				// Ensure that the scope ID is always ignored.
				testEqualsScopeId(obj, set);
			}
			catch (Exception e) {
				fail("Unexpected exception: " + e);
			}
		}
	}

	/**
	 * Ensure that {@link IpcInetAddress#equals(Object)} and
	 * {@link IpcInetAddress#hashCode()} ignore the scope ID in an
	 * {@link Inet6Address} object.
	 *
	 * @param obj	An {@link IpcInetAddress} object to be tested.
	 * @param set	Set of {@link IpcInetAddress} which contains
	 *		{@code obj}.
	 * @throws Exception
	 *	This should never happen.
	 */
	private void testEqualsScopeId(IpcInetAddress obj,
				       HashSet<IpcInetAddress> set)
		throws Exception
	{
		for (int scope = 1; scope <= 10; scope++) {
			String text = obj.toString() + "%" + scope;
			Inet6Address iaddr =
				(Inet6Address)InetAddress.getByName(text);
			assertEquals(scope, iaddr.getScopeId());

			IpcInetAddress newobj = IpcInetAddress.create(iaddr);
			assertEquals(obj, newobj);
			assertEquals(obj.hashCode(), newobj.hashCode());

			assertFalse(set.add(newobj));
		}
	}

	/**
	 * Ensure that the specified object has the specified value.
	 *
	 * @param obj		An object to be tested.
	 * @param addr		A required raw IPv6 address.
	 */
	private void checkValue(IpcInetAddress obj, byte[] addr)
	{
		InetAddress iaddr = obj.getValue();
		assertTrue(iaddr instanceof Inet6Address);

		Inet6Address i6addr = (Inet6Address)iaddr;
		assertEquals(0, i6addr.getScopeId());

		byte[] raw = iaddr.getAddress();
		assertEquals(16, raw.length);
		assertTrue(Arrays.equals(raw, addr));

		assertEquals(IpcDataUnit.IPV6, obj.getType());
		assertTrue(obj instanceof IpcInet6Address);
	}
}
