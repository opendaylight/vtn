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
 *   Unit test class for {@link IpcInt32}.
 * </p>
 */
public class IpcInt32Test extends TestBase
{
	/**
	 * Random instance for Unit test.
	 */
	private Random rand;

	/**
	 * Dictionary of boundary values.
	 */
	static int boundary_values[] = {
		Integer.MIN_VALUE,
		-1,
		0,
		1,
		Integer.MAX_VALUE,
	};

	/**
	 * Create JUnit test case for {@link IpcInt32}.
	 *
	 * @param name	The test name.
	 */
	public IpcInt32Test(String name)
	{
		super(name);
		rand = new Random();
	}

	/**
	 * Test case for {@link IpcInt32#IpcInt32(int)}.
	 */
	public void testCtorInt()
	{
		for (int loop = 0; loop < 10000; loop++) {
			int i = getTestValue(loop);
			IpcInt32 obj = new IpcInt32(i);
			checkValue(obj, i);
		}
	}

	/**
	 * Test case for {@link IpcInt32#IpcInt32(String)}.
	 */
	public void testCtorString()
	{
		for (int loop = 0; loop < 10000; loop++) {
			int i = getTestValue(loop);

			// Decimal
			String str = Integer.toString(i);
			IpcInt32 obj = new IpcInt32(i);
			checkValue(obj, i);

			// Octal
			if (i == Integer.MIN_VALUE) {
				str = "-0" + Integer.toOctalString(i);
			}
			else if (i < 0) {
				str = "-0" + Integer.toOctalString(-i);
			}
			else {
				str = "0" + Integer.toOctalString(i);
			}
			obj = new IpcInt32(str);
			checkValue(obj, i);

			// Hexadecimal
			String hex, sign;
			if (i == Integer.MIN_VALUE) {
				hex = Integer.toHexString(i);
				sign = "-";
			}
			else if (i < 0) {
				hex = Integer.toHexString(-i);
				sign = "-";
			}
			else {
				hex = Integer.toHexString(i);
				sign = "";
			}

			str = sign + "0x" + hex;
			obj = new IpcInt32(str);
			checkValue(obj, i);

			str = str.toUpperCase();
			obj = new IpcInt32(str);
			checkValue(obj, i);

			str = sign + "#" + hex;
			obj = new IpcInt32(str);
			checkValue(obj, i);
		}

		// Invalid String.
		try {
			new IpcInt32(null);
			needException();
		}
		catch (NullPointerException e) {}

		String[] invalid = {
			// no length.
			"",
			// out of range for each expression.
			"2147483648",
			"-2147483649",
			"020000000000",
			"-020000000001",
			"0x80000000",
			"-0x80000001",
			// miss expressions.
			"0900000",
			"0xz",
			"x80000",
			// normal string
			"string",
			// type suffix expression. (not accept)
			"654321L",
			"654321 L",
		};

		for (int idx = 0; idx < invalid.length; idx++) {
			try {
				new IpcInt32(invalid[idx]);
				needException(invalid[idx]);
			}
			catch (NumberFormatException e) {}
		}
	}

	/**
	 * Test case for {@link IpcInt32#compareTo(IpcInt32)}.
	 */
	public void testCompareTo()
	{
		for (int loop = 0; loop < 10000; loop++) {
			int i = getTestValue(loop);
			int j = getTestValue(loop);

			IpcInt32 obji = new IpcInt32(i);
			IpcInt32 objj = new IpcInt32(j);
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
	 * Test case for {@link IpcInt32#equals(Object)} and
	 * {@link IpcInt32#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<IpcInt32> set = new HashSet<IpcInt32>();
		HashSet<Integer> iset = new HashSet<Integer>();

		for (int loop = 0; loop < 10000; loop++) {
			int i = getTestValue(loop);
			IpcInt32 obj = new IpcInt32(i);
			assertTrue(obj.equals(obj));
			assertFalse(obj.equals(null));
			assertFalse(obj.equals(new Object()));

			Integer iobj = new Integer(i);
			if (iset.add(iobj)) {
				assertTrue(set.add(obj));
				assertFalse(set.add(obj));
			}

			obj = new IpcInt32(i);
			assertFalse(set.add(obj));
		}
	}

	/**
	 * Ensure that the specified object has the specified value.
	 *
	 * @param obj		An object to be tested.
	 * @param value		Required value
	 */
	private void checkValue(IpcInt32 obj, int value)
	{
		assertEquals((byte)value, obj.byteValue());
		assertEquals((short)value, obj.shortValue());
		assertEquals(value, obj.intValue());
		assertEquals((long)value, obj.longValue());
		assertEquals((float)value, obj.floatValue());
		assertEquals((double)value, obj.doubleValue());

		assertEquals(Integer.toString(value), obj.toString());

		assertEquals(IpcDataUnit.INT32, obj.getType());
	}

	/**
	 * <p>
	 * Detect value for each test. 
	 * </p>
	 * <p>
	 *   This method uses for detcting input values for each unit test.
	 * </p>
	 *
	 * @param	idx	index of using values.
	 * @return	return index-th boundary value when index is in 
	 *		the dictionary of doundary values. 
	 *		othewise, return a randam number.
	 */
	private int getTestValue(int idx)
	{
		if (idx < boundary_values.length) {
			return boundary_values[idx];
		} else {
			return rand.nextInt();
		}
	}
}
