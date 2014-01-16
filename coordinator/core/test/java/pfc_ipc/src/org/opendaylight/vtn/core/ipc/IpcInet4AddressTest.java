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

/**
 * <p>
 *   Unit test class for {@link IpcInet4Address}.
 * </p>
 */
public class IpcInet4AddressTest extends TestBase
{
	/**
	 * Define boundary address for IPv4.
	 */
	private static byte[][] boundary_addrs = {
		// ANY 
		{ (byte)  0, (byte)  0, (byte)  0, (byte)  0 },
		// class A network/broadcast/private
		{ (byte)  9, (byte)  0, (byte)  0, (byte)  0 },
		{ (byte)  9, (byte)255, (byte)255, (byte)255 },
		{ (byte) 10, (byte)  0, (byte)  0, (byte)  0 },
		{ (byte) 10, (byte)255, (byte)255, (byte)255 },
		// loopback
		{ (byte)127, (byte)  0, (byte)  0, (byte)  1 },
		{ (byte)127, (byte)255, (byte)255, (byte)255 },
		// class B network/broadcast/private
		{ (byte)128, (byte)  0, (byte)  0, (byte)  0 },
		{ (byte)128, (byte)  0, (byte)255, (byte)255 },
		{ (byte)172, (byte) 16, (byte)  0, (byte)  0 },
		{ (byte)172, (byte) 31, (byte)255, (byte)255 },
		// class C network/broadcast/private
		{ (byte)192, (byte)  0, (byte)  0, (byte)  0 },
		{ (byte)192, (byte)  0, (byte)  0, (byte)255 },
		{ (byte)192, (byte)168, (byte)  0, (byte)  0 },
		{ (byte)192, (byte)168, (byte)255, (byte)255 },
		// class D
		{ (byte)224, (byte)  0, (byte)  0, (byte)  0 },
		{ (byte)239, (byte)255, (byte)255, (byte)255 },
		// class E
		{ (byte)240, (byte)  0, (byte)  0, (byte)  0 },
		{ (byte)255, (byte)255, (byte)255, (byte)255 },
	};

	/**
	 * Create JUnit test case for {@link IpcInet4Address}.
	 *
	 * @param name	The test name.
	 */
	public IpcInet4AddressTest(String name)
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
			byte[] raw = new byte[4];
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

		// boundary check
		for (int idx = 0; idx < boundary_addrs.length; idx++) {
			try {
				InetAddress iaddr =
					InetAddress.getByAddress(boundary_addrs[idx]);
				IpcInetAddress obj =
					IpcInetAddress.create(iaddr);
				checkValue(obj, boundary_addrs[idx]);

				IpcInetAddress objInet4 =
					IpcInet4Address.create(iaddr);
				checkValue(objInet4, boundary_addrs[idx]);

				IpcInetAddress objInet6 =
					IpcInet6Address.create(iaddr);
				checkValue(objInet6, boundary_addrs[idx]);
			}
			catch (Exception e) {
				fail("Unexpected exception: " + e);
			}
		}

		// Specify null.
		try {
			IpcInetAddress.create((InetAddress)null);
			needException();
		}
		catch (NullPointerException e) {}
		try {
			IpcInet4Address.create((InetAddress)null);
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

		for (int loop = 0; loop < 0x10000; loop++) {
			byte[] raw = new byte[4];
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


		// boundary check
		for (int idx = 0; idx < boundary_addrs.length; idx++) {
			try {
				IpcInetAddress obj =
					IpcInetAddress.create(boundary_addrs[idx]);
				checkValue(obj, boundary_addrs[idx]);

				IpcInetAddress objInet4 =
					IpcInet4Address.create(boundary_addrs[idx]);
				checkValue(objInet4, boundary_addrs[idx]);

				IpcInetAddress objInet6 =
					IpcInet6Address.create(boundary_addrs[idx]);
				checkValue(objInet6, boundary_addrs[idx]);
			}
			catch (Exception e) {
				fail("Unexpected exception: " + e);
			}
		}

		// Specify null.
		try {
			IpcInetAddress.create((byte[])null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class, null);
		}

		try {
			IpcInet4Address.create((byte[])null);
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

			byte[] raw = new byte[i];
			try {
				IpcInetAddress.create(raw);
				needException(i);
			}
			catch (Exception e) {
				assertEquals(IllegalArgumentException.class,
					     e.getClass());
			}
			try {
				IpcInet4Address.create(raw);
				needException(i);
			}
			catch (Exception e) {
				assertEquals(IllegalArgumentException.class,
					     e.getClass());
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

		IpcInetAddress obj6 = null;
		try {
			InetAddress addr6 =
				InetAddress.getByName("ffff:abcd::123");
			obj6 = IpcInetAddress.create(addr6);
		}
		catch (Exception e) {
			fail("Unexpected exception: " + e);
		}

		for (int loop = 0; loop < 0x10000; loop++) {
			byte[] raw = new byte[4];
			rand.nextBytes(raw);

			try {
				IpcInetAddress obj =
					IpcInetAddress.create(raw);
				assertTrue(obj.equals(obj));
				assertFalse(obj.equals(null));
				assertFalse(obj.equals(new Object()));
				assertFalse(obj.equals(obj6));

				InetAddress iaddr = obj.getValue();
				if (iset.add(iaddr)) {
					assertTrue(set.add(obj));
					assertFalse(set.add(obj));
				}
			
				obj = IpcInetAddress.create(raw);
				assertFalse(set.add(obj));
			}
			catch (Exception e) {
				fail("Unexpected exception: " + e);
			}
		}
	}

	/**
	 * Ensure that the specified object has the specified value.
	 *
	 * @param obj		An object to be tested.
	 * @param addr		A required raw IPv4 address.
	 */
	private void checkValue(IpcInetAddress obj, byte[] addr)
	{
		InetAddress iaddr = obj.getValue();
		assertTrue(iaddr instanceof Inet4Address);

		byte[] raw = iaddr.getAddress();
		assertEquals(4, raw.length);
		assertTrue(Arrays.equals(raw, addr));

		assertEquals(IpcDataUnit.IPV4, obj.getType());
		assertTrue(obj instanceof IpcInet4Address);

		String text = (addr[0] & 0xff) + "." + (addr[1] & 0xff) + "." +
			(addr[2] & 0xff) + "." + (addr[3] & 0xff);
		assertEquals(text, obj.toString());
	}
}
