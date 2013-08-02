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
 *   Unit test class for {@link IpcInt8}.
 * </p>
 */
public class IpcInt8Test extends TestBase
{
	/**
	 * Create JUnit test case for {@link IpcInt8}.
	 *
	 * @param name	The test name.
	 */
	public IpcInt8Test(String name)
	{
		super(name);
	}

	/**
	 * Test case for {@link IpcInt8#IpcInt8(byte)}.
	 */
	public void testCtorByte()
	{
		for (int i = Byte.MIN_VALUE; i <= Byte.MAX_VALUE; i++) {
			byte b = (byte)i;
			IpcInt8 obj = new IpcInt8(b);
			checkValue(obj, b);
		}
	}

	/**
	 * Test case for {@link IpcInt8#IpcInt8(String)}.
	 */
	public void testCtorString()
	{
		for (int i = Byte.MIN_VALUE; i <= Byte.MAX_VALUE; i++) {
			// Decimal
			String s = Integer.toString(i);
			IpcInt8 obj = new IpcInt8(s);
			checkValue(obj, (byte)i);

			// Octal
			if (i < 0) {
				s = "-0" + Integer.toOctalString(-i);
			}
			else {
				s = "0" + Integer.toOctalString(i);
			}
			obj = new IpcInt8(s);
			checkValue(obj, (byte)i);

			// Hexadecimal
			String hex, sign;
			if (i < 0) {
				hex = Integer.toHexString(-i);
				sign = "-";
			}
			else {
				hex = Integer.toHexString(i);
				sign = "";
			}

			s = sign + "0x" + hex;
			obj = new IpcInt8(s);
			checkValue(obj, (byte)i);

			s = s.toUpperCase();
			obj = new IpcInt8(s);
			checkValue(obj, (byte)i);

			s = sign + "#" + hex;
			obj = new IpcInt8(s);
			checkValue(obj, (byte)i);
		}

		// Invalid String.
		try {
			new IpcInt8(null);
			needException();
		}
		catch (NullPointerException e) {}

		String[] invalid = {
			// no length.
			"",
			// out of range for each expression.
			"128",
			"-129",
			"0200",
			"-0201",
			"0x80",
			"-0x81",
			// miss expressions.
			"090",
			"0xg",
			"x01",
			// normal string
			"string",
			// type suffix expression. (not accept)
			"127L",
			"127 L",
		};

		for (int idx = 0; idx < invalid.length; idx++) {
			try {
				new IpcInt8(invalid[idx]);
				needException(invalid[idx]);
			}
			catch (NumberFormatException e) {}
		}
	}

	/**
	 * Test case for {@link IpcInt8#compareTo(IpcInt8)}.
	 */
	public void testCompareTo()
	{
		for (int i = Byte.MIN_VALUE; i <= Byte.MAX_VALUE; i++) {
			for (int j = Byte.MIN_VALUE; j <= Byte.MAX_VALUE;
			     j++) {
				byte  bi = (byte)i;
				byte  bj = (byte)j;

				IpcInt8 obji = new IpcInt8(bi);
				IpcInt8 objj = new IpcInt8(bj);
				assertEquals(0, obji.compareTo(obji));
				assertEquals(0, objj.compareTo(objj));

				int result = obji.compareTo(objj);
				if (bi == bj) {
					assertEquals(0, result);
				}
				else if (bi < bj) {
					assertTrue(result < 0);
				}
				else {
					assertTrue(result > 0);
				}
			}
		}
	}

	/**
	 * Test case for {@link IpcInt8#equals(Object)} and
	 * {@link IpcInt8#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<IpcInt8> set = new HashSet<IpcInt8>();

		for (int i = Byte.MIN_VALUE; i <= Byte.MAX_VALUE; i++) {
			byte b = (byte)i;
			IpcInt8 obj = new IpcInt8(b);
			assertTrue(obj.equals(obj));
			assertFalse(obj.equals(null));
			assertFalse(obj.equals(new Object()));

			assertTrue(set.add(obj));
			assertFalse(set.add(obj));

			IpcInt8 obj2 = new IpcInt8(b);
			assertTrue(obj.equals(obj2));
			assertEquals(obj, obj2);
			assertEquals(obj.hashCode(), obj2.hashCode());
			assertFalse(set.add(obj2));
		}
	}

	/**
	 * Ensure that the specified object has the specified value.
	 *
	 * @param obj		An object to be tested.
	 * @param value		Required value
	 */
	private void checkValue(IpcInt8 obj, byte value)
	{
		assertEquals(value, obj.byteValue());
		assertEquals((short)value, obj.shortValue());
		assertEquals((int)value, obj.intValue());
		assertEquals((long)value, obj.longValue());
		assertEquals((float)value, obj.floatValue());
		assertEquals((double)value, obj.doubleValue());

		assertEquals(Byte.toString(value), obj.toString());

		assertEquals(IpcDataUnit.INT8, obj.getType());
	}
}
