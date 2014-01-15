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
 *   Unit test class for {@link IpcInt16}.
 * </p>
 */
public class IpcInt16Test extends TestBase
{
	/**
	 * Create JUnit test case for {@link IpcInt16}.
	 *
	 * @param name	The test name.
	 */
	public IpcInt16Test(String name)
	{
		super(name);
	}

	/**
	 * Test case for {@link IpcInt16#IpcInt16(short)}.
	 */
	public void testCtorShort()
	{
		for (int i = Short.MIN_VALUE; i <= Short.MAX_VALUE; i++) {
			short s = (short)i;
			IpcInt16 obj = new IpcInt16(s);
			checkValue(obj, s);
		}
	}

	/**
	 * Test case for {@link IpcInt16#IpcInt16(String)}.
	 */
	public void testCtorString()
	{
		for (int i = Short.MIN_VALUE; i <= Short.MAX_VALUE; i++) {
			// Decimal
			String str = Integer.toString(i);
			IpcInt16 obj = new IpcInt16(str);
			checkValue(obj, (short)i);

			// Octal
			if (i < 0) {
				str = "-0" + Integer.toOctalString(-i);
			}
			else {
				str = "0" + Integer.toOctalString(i);
			}
			obj = new IpcInt16(str);
			checkValue(obj, (short)i);

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

			str = sign + "0x" + hex;
			obj = new IpcInt16(str);
			checkValue(obj, (short)i);

			str = str.toUpperCase();
			obj = new IpcInt16(str);
			checkValue(obj, (short)i);

			str = sign + "#" + hex;
			obj = new IpcInt16(str);
			checkValue(obj, (short)i);
		}

		// Invalid String.
		try {
			new IpcInt16(null);
			needException();
		}
		catch (NullPointerException e) {}

		String[] invalid = {
			// no length.
			"",
			// out of range for each expressions.
			"32768",
			"-32769",
			"0100000",
			"-0100001",
			"0x8000",
			"-0x8001",
			// miss expressions.
			"09000",
			"0xkkk",
			"x100",
			// normal string
			"string",
			// type suffix expression. (not accept)
			"30000L",
			"30000 L",
		};

		for (int idx = 0; idx < invalid.length; idx++) {
			try {
				new IpcInt16(invalid[idx]);
				needException(invalid[idx]);
			}
			catch (NumberFormatException e) {}
		}
	}

	/**
	 * Test case for {@link IpcInt16#compareTo(IpcInt16)}.
	 */
	public void testCompareTo()
	{
		Random rand = new Random();

		for (int loop = 0; loop < 0x100000; loop++) {
			int i = rand.nextInt() & 0xffff;
			int j = rand.nextInt() & 0xffff;
			short  si = (short)i;
			short  sj = (short)j;

			IpcInt16 obji = new IpcInt16(si);
			IpcInt16 objj = new IpcInt16(sj);
			assertEquals(0, obji.compareTo(obji));
			assertEquals(0, objj.compareTo(objj));

			int result = obji.compareTo(objj);
			if (si == sj) {
				assertEquals(0, result);
			}
			else if (si < sj) {
				assertTrue(result < 0);
			}
			else {
				assertTrue(result > 0);
			}
		}
	}

	/**
	 * Test case for {@link IpcInt16#equals(Object)} and
	 * {@link IpcInt16#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<IpcInt16> set = new HashSet<IpcInt16>();

		for (int i = Short.MIN_VALUE; i <= Short.MAX_VALUE; i++) {
			short s = (short)i;
			IpcInt16 obj = new IpcInt16(s);
			assertTrue(obj.equals(obj));
			assertFalse(obj.equals(null));
			assertFalse(obj.equals(new Object()));

			assertTrue(set.add(obj));
			assertFalse(set.add(obj));
		}

		for (int i = Short.MIN_VALUE; i <= Short.MAX_VALUE; i++) {
			short s = (short)i;
			IpcInt16 obj = new IpcInt16(s);
			assertFalse(set.add(obj));
		}
	}

	/**
	 * Ensure that the specified object has the specified value.
	 *
	 * @param obj		An object to be tested.
	 * @param value		Required value
	 */
	private void checkValue(IpcInt16 obj, short value)
	{
		assertEquals((byte)value, obj.byteValue());
		assertEquals(value, obj.shortValue());
		assertEquals((int)value, obj.intValue());
		assertEquals((long)value, obj.longValue());
		assertEquals((float)value, obj.floatValue());
		assertEquals((double)value, obj.doubleValue());

		assertEquals(Short.toString(value), obj.toString());

		assertEquals(IpcDataUnit.INT16, obj.getType());
	}
}
