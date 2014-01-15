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
 *   Unit test class for {@link IpcInt64}.
 * </p>
 */
public class IpcInt64Test extends TestBase
{
	/**
	 * Random instance for Unit test.
	 */
	private Random rand;

	/**
	 * Dictionary of boundary values.
	 */
	static long boundary_values[] = {
		Long.MIN_VALUE,
		(long)Integer.MIN_VALUE - 1,
		Integer.MIN_VALUE,
		-1,
		0,
		1,
		Integer.MAX_VALUE,
		(long)Integer.MAX_VALUE + 1,
		Long.MAX_VALUE,
	};

	/**
	 * Create JUnit test case for {@link IpcInt64}.
	 *
	 * @param name	The test name.
	 */
	public IpcInt64Test(String name)
	{
		super(name);
		rand = new Random();
	}

	/**
	 * Test case for {@link IpcInt64#IpcInt64(long)}.
	 */
	public void testCtorLong()
	{
		for (int loop = 0; loop < 10000; loop++) {
			long l = getTestValue(loop);
			IpcInt64 obj = new IpcInt64(l);
			checkValue(obj, l);
		}
	}

	/**
	 * Test case for {@link IpcInt64#IpcInt64(String)}.
	 */
	public void testCtorString()
	{
		for (int loop = 0; loop < 10000; loop++) {
			long l = getTestValue(loop);

			// Decimal
			String str = Long.toString(l);
			IpcInt64 obj = new IpcInt64(l);
			checkValue(obj, l);

			// Octal
			if (l == Long.MIN_VALUE) {
				str = "-0" + Long.toOctalString(l);
			}
			else if (l < 0) {
				str = "-0" + Long.toOctalString(-l);
			}
			else {
				str = "0" + Long.toOctalString(l);
			}
			obj = new IpcInt64(str);
			checkValue(obj, l);

			// Hexadecimal
			String hex, sign;
			if (l == Long.MIN_VALUE) {
				hex = Long.toHexString(l);
				sign = "-";
			}
			else if (l < 0) {
				hex = Long.toHexString(-l);
				sign = "-";
			}
			else {
				hex = Long.toHexString(l);
				sign = "";
			}

			str = sign + "0x" + hex;
			obj = new IpcInt64(str);
			checkValue(obj, l);

			str = str.toUpperCase();
			obj = new IpcInt64(str);
			checkValue(obj, l);

			str = sign + "#" + hex;
			obj = new IpcInt64(str);
			checkValue(obj, l);
		}

		// Invalid String.
		try {
			new IpcInt64(null);
			needException();
		}
		catch (NullPointerException e) {}

		String[] invalid = {
			// no length.
			"",
			// out of range for each expression.
			"9223372036854775808",
			"-9223372036854775809",
			"01000000000000000000000",
			"-01000000000000000000001",
			"0x8000000000000000",
			"-0x8000000000000001",
			"-0x8000000000000001",
			// miss expressions.
			"090000000000",
			"0xX",
			"x8000000",
			// normal string
			"string",
			// type suffix expression. (not accept)
			"12345678901L",
			"12345678901 L",
		};

		for (int idx = 0; idx < invalid.length; idx++) {
			try {
				new IpcInt64(invalid[idx]);
				needException(invalid[idx]);
			}
			catch (NumberFormatException e) {}
		}
	}

	/**
	 * Test case for {@link IpcInt64#compareTo(IpcInt64)}.
	 */
	public void testCompareTo()
	{
		for (int loop = 0; loop < 10000; loop++) {
			long i = getTestValue(loop);
			long j = getTestValue(loop);

			IpcInt64 obji = new IpcInt64(i);
			IpcInt64 objj = new IpcInt64(j);
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
	 * Test case for {@link IpcInt64#equals(Object)} and
	 * {@link IpcInt64#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<IpcInt64> set = new HashSet<IpcInt64>();
		HashSet<Long> lset = new HashSet<Long>();

		for (int loop = 0; loop < 10000; loop++) {
			long l = getTestValue(loop);
			IpcInt64 obj = new IpcInt64(l);
			assertTrue(obj.equals(obj));
			assertFalse(obj.equals(null));
			assertFalse(obj.equals(new Object()));

			Long lobj = new Long(l);
			if (lset.add(lobj)) {
				assertTrue(set.add(obj));
				assertFalse(set.add(obj));
			}

			obj = new IpcInt64(l);
			assertFalse(set.add(obj));
		}
	}

	/**
	 * Ensure that the specified object has the specified value.
	 *
	 * @param obj		An object to be tested.
	 * @param value		Required value
	 */
	private void checkValue(IpcInt64 obj, long value)
	{
		assertEquals((byte)value, obj.byteValue());
		assertEquals((short)value, obj.shortValue());
		assertEquals((int)value, obj.intValue());
		assertEquals(value, obj.longValue());
		assertEquals((float)value, obj.floatValue());
		assertEquals((double)value, obj.doubleValue());

		assertEquals(Long.toString(value), obj.toString());

		assertEquals(IpcDataUnit.INT64, obj.getType());
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
	private long getTestValue(int idx)
	{
		if (idx < boundary_values.length) {
			return boundary_values[idx];
		} else {
			return rand.nextLong();
		}
	}
}
