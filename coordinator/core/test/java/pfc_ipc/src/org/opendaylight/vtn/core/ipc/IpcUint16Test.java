/*
 * Copyright (c) 2012-2014 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.ipc;

import java.util.HashSet;
import java.util.Random;

/**
 * <p>
 *   Unit test class for {@link IpcUint16}.
 * </p>
 */
public class IpcUint16Test extends TestBase
{
	/**
	 * Create JUnit test case for {@link IpcUint16}.
	 *
	 * @param name	The test name.
	 */
	public IpcUint16Test(String name)
	{
		super(name);
	}

	/**
	 * Test case for {@link IpcUint16#IpcUint16(short)}.
	 */
	public void testCtorShort()
	{
		for (int i = 0; i <= 0xffff; i++) {
			short s = (short)i;
			IpcUint16 obj = new IpcUint16(s);
			checkValue(obj, s);
		}
	}

	/**
	 * Test case for {@link IpcUint16#IpcUint16(int)}.
	 */
	public void testCtorInt()
	{
		for (int i = 0; i <= 0xffff; i++) {
			IpcUint16 obj = new IpcUint16(i);
			checkValue(obj, (short)i);
		}

		// Invalid parameter.
		for (int i = -0x10000; i < 0; i++) {
			try {
				new IpcUint16(i);
				needException();
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Out of valid range: " + i);
			}
		}

		for (int i = 0x10000; i <= 0x30000; i += 10) {
			try {
				new IpcUint16(i);
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
	 * Test case for {@link IpcUint16#IpcUint16(String)}.
	 */
	public void testCtorString()
	{
		for (int i = 0; i <= 0xffff; i++) {
			// Decimal
			String s = Integer.toString(i);
			IpcUint16 obj = new IpcUint16(s);
			checkValue(obj, (short)i);

			// Octal
			s = "0" + Integer.toOctalString(i);
			obj = new IpcUint16(s);
			checkValue(obj, (short)i);

			// Hexadecimal
			String hex = Integer.toHexString(i);
			s = "0x" + hex;
			obj = new IpcUint16(s);
			checkValue(obj, (short)i);

			s = s.toUpperCase();
			obj = new IpcUint16(s);
			checkValue(obj, (short)i);

			s = "#" + hex;
			obj = new IpcUint16(s);
			checkValue(obj, (short)i);
		}

		// Invalid String.
		try {
			new IpcUint16(null);
			needException();
		}
		catch (NullPointerException e) {}

		String[] invalid = {
			// no length.
			"",
			// out of range for each expression.
			"65536",
			"-1",
			"0200000",
			"-01",
			"0x10000",
			// miss expressions.
			"08765",
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
				new IpcUint16(invalid[idx]);
				needException(invalid[idx]);
			}
			catch (NumberFormatException e) {}
		}
	}

	/**
	 * Test case for {@link IpcUint16#compareTo(IpcUint16)}.
	 */
	public void testCompareTo()
	{
		Random rand = new Random();

		for (int loop = 0; loop < 10000; loop++) {
			int i = rand.nextInt() & 0xffff;
			int j = rand.nextInt() & 0xffff;
			IpcUint16 obji = new IpcUint16((short)i);
			IpcUint16 objj = new IpcUint16((short)j);
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

	/**
	 * Test case for {@link IpcUint16#equals(Object)} and
	 * {@link IpcUint16#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<IpcUint16> set = new HashSet<IpcUint16>();

		for (int i = 0; i <= 0xffff; i++) {
			short s = (short)i;
			IpcUint16 obj = new IpcUint16(s);
			assertTrue(obj.equals(obj));
			assertFalse(obj.equals(null));
			assertFalse(obj.equals(new Object()));

			assertTrue(set.add(obj));
			assertFalse(set.add(obj));
		}

		for (int i = 0; i <= 0xffff; i++) {
			short s = (short)i;
			IpcUint16 obj = new IpcUint16(s);
			assertFalse(set.add(obj));
		}
	}

	/**
	 * Ensure that the specified object has the specified value.
	 *
	 * @param obj		An object to be tested.
	 * @param value		Required value
	 */
	private void checkValue(IpcUint16 obj, short value)
	{
		int i = (int)value;
		if (i < 0) {
			i += 65536;
		}

		assertEquals((byte)i, obj.byteValue());
		assertEquals(value, obj.shortValue());
		assertEquals(i, obj.intValue());
		assertEquals((long)i, obj.longValue());
		assertEquals((float)i, obj.floatValue());
		assertEquals((double)i, obj.doubleValue());

		assertEquals(Integer.toString(i), obj.toString());

		assertEquals(IpcDataUnit.UINT16, obj.getType());
	}
}
