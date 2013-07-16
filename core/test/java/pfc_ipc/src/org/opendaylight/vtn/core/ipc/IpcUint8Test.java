/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import java.util.HashSet;

/**
 * <p>
 *   Unit test class for {@link IpcUint8}.
 * </p>
 */
public class IpcUint8Test extends TestBase
{
	/**
	 * Create JUnit test case for {@link IpcUint8}.
	 *
	 * @param name	The test name.
	 */
	public IpcUint8Test(String name)
	{
		super(name);
	}

	/**
	 * Test case for {@link IpcUint8#IpcUint8(byte)}.
	 */
	public void testCtorByte()
	{
		for (int i = 0; i <= 0xff; i++) {
			byte b = (byte)i;
			IpcUint8 obj = new IpcUint8(b);
			checkValue(obj, b);
		}
	}

	/**
	 * Test case for {@link IpcUint8#IpcUint8(int)}.
	 */
	public void testCtorInt()
	{
		for (int i = 0; i <= 0xff; i++) {
			IpcUint8 obj = new IpcUint8(i);
			checkValue(obj, (byte)i);
		}

		// Invalid parameter.
		for (int i = -10000; i < 0; i++) {
			try {
				new IpcUint8(i);
				needException(i);
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Out of valid range: " + i);
			}
		}

		for (int i = 0x100; i <= 0x1000; i++) {
			try {
				new IpcUint8(i);
				needException(i);
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Out of valid range: " + i);
			}
		}
	}

	/**
	 * Test case for {@link IpcUint8#IpcUint8(String)}.
	 */
	public void testCtorString()
	{
		for (int i = 0; i <= 0xff; i++) {
			// Decimal
			String s = Integer.toString(i);
			IpcUint8 obj = new IpcUint8(s);
			checkValue(obj, (byte)i);

			// Octal
			s = "0" + Integer.toOctalString(i);
			obj = new IpcUint8(s);
			checkValue(obj, (byte)i);

			// Hexadecimal
			String hex = Integer.toHexString(i);
			s = "0x" + hex;
			obj = new IpcUint8(s);
			checkValue(obj, (byte)i);

			s = s.toUpperCase();
			obj = new IpcUint8(s);
			checkValue(obj, (byte)i);

			s = "#" + hex;
			obj = new IpcUint8(s);
			checkValue(obj, (byte)i);
		}

		// Invalid String.
		try {
			new IpcUint8(null);
			needException();
		}
		catch (NullPointerException e) {}

		String[] invalid = {
			// no length.
			"",
			// out of range for each expression.
			"-1",
			"-12",
			"256",
			"0400",
			"-01",
			"0x100",
			"-0x1",
			// miss expressions.
			"090",
			"0xg",
			"x1",
			// normal string
			"string",
			// type suffix expression. (not accept)
			"127L",
			"127 L",
		};

		for (int idx = 0; idx < invalid.length; idx++) {
			try {
				new IpcUint8(invalid[idx]);
				needException(invalid[idx]);
			}
			catch (NumberFormatException e) {}
		}
	}

	/**
	 * Test case for {@link IpcUint8#compareTo(IpcUint8)}.
	 */
	public void testCompareTo()
	{
		for (int i = 0; i <= 0xff; i++) {
			for (int j = 0; j <= 0xff; j++) {
				byte  bi = (byte)i;
				byte  bj = (byte)j;

				IpcUint8 obji = new IpcUint8(bi);
				IpcUint8 objj = new IpcUint8(bj);
				assertEquals(0, obji.compareTo(obji));
				assertEquals(0, objj.compareTo(objj));

				int result = obji.compareTo(objj);
				if (i == j) {
					assertEquals(0, result);
				}
				else if (i < j) {
					assertTrue(result < 0);
				}
				else {
					assertTrue(result > 0);
				}
			}
		}
	}

	/**
	 * Test case for {@link IpcUint8#equals(Object)} and
	 * {@link IpcUint8#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<IpcUint8> set = new HashSet<IpcUint8>();

		for (int i = 0; i <= 0xff; i++) {
			byte b = (byte)i;
			IpcUint8 obj = new IpcUint8(b);
			assertTrue(obj.equals(obj));
			assertFalse(obj.equals(null));
			assertFalse(obj.equals(new Object()));

			assertTrue(set.add(obj));
			assertFalse(set.add(obj));
		}

		for (int i = 0; i <= 0xff; i++) {
			byte b = (byte)i;
			IpcUint8 obj = new IpcUint8(b);
			assertFalse(set.add(obj));
		}
	}

	/**
	 * Ensure that the specified object has the specified value.
	 *
	 * @param obj		An object to be tested.
	 * @param value		Required value
	 */
	private void checkValue(IpcUint8 obj, byte value)
	{
		int i = (int)value;
		if (i < 0) {
			i += 256;
		}

		assertEquals(value, obj.byteValue());
		assertEquals((short)i, obj.shortValue());
		assertEquals(i, obj.intValue());
		assertEquals((long)i, obj.longValue());
		assertEquals((float)i, obj.floatValue());
		assertEquals((double)i, obj.doubleValue());

		assertEquals(Integer.toString(i), obj.toString());

		assertEquals(IpcDataUnit.UINT8, obj.getType());
	}
}
