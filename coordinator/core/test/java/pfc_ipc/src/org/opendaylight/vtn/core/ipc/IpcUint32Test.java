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
 *   Unit test class for {@link IpcUint32}.
 * </p>
 */
public class IpcUint32Test extends TestBase
{
	/**
	 * Random instance for Unit test.
	 */
	private Random rand;


	/**
	 * Dictionary of boundary values.
	 */
	static int boundary_int_values[] = {
		Integer.MIN_VALUE,
		-1,
		0,
		1,
		Integer.MAX_VALUE,
	};


	/**
	 * Dictionary of boundary values for type long.
	 */
	static long boundary_long_values[] = {
		0,
		1,
		Integer.MAX_VALUE,
		(long)Integer.MAX_VALUE + 1,
		(long)Integer.MAX_VALUE * 2 + 1,
	};


	/**
	 * Create JUnit test case for {@link IpcUint32}.
	 *
	 * @param name	The test name.
	 */
	public IpcUint32Test(String name)
	{
		super(name);
		rand = new Random();
	}

	/**
	 * Test case for {@link IpcUint32#IpcUint32(int)}.
	 */
	public void testCtorInt()
	{
		for (int loop = 0; loop < 10000; loop++) {
			int i = getTestValueInt(loop);
			IpcUint32 obj = new IpcUint32(i);
			checkValue(obj, i);
		}
	}

	/**
	 * Test case for {@link IpcUint32#IpcUint32(long)}.
	 */
	public void testCtorLong()
	{
		for (int loop = 0; loop < 10000; loop++) {
			long l = getTestValueLong(loop);
			IpcUint32 obj = new IpcUint32(l);
			checkValue(obj, (int)l);
		}

		// Invalid parameter.
		for (long l = -10000L; l < 0L; l++) {
			try {
				new IpcUint32(l);
				needException();
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Out of valid range: " + l);
			}
		}

		for (long l = 0x100000000L; l <= 0x100020000L; l += 10) {
			try {
				new IpcUint32(l);
				needException(l);
			}
			catch (Exception e) {
				checkException
					(e, IllegalArgumentException.class,
					 "Out of valid range: " + l);
			}
		}
	}

	/**
	 * Test case for {@link IpcUint32#IpcUint32(String)}.
	 */
	public void testCtorString()
	{
		for (int loop = 0; loop < 10000; loop++) {
			long l = getTestValueLong(loop);

			// Decimal
			String s = Long.toString(l);
			IpcUint32 obj = new IpcUint32(s);
			checkValue(obj, (int)l);

			// Octal
			s = "0" + Long.toOctalString(l);
			obj = new IpcUint32(s);
			checkValue(obj, (int)l);

			// Hexadecimal
			String hex = Long.toHexString(l);
			s = "0x" + hex;
			obj = new IpcUint32(s);
			checkValue(obj, (int)l);

			s = s.toUpperCase();
			obj = new IpcUint32(s);
			checkValue(obj, (int)l);

			s = "#" + hex;
			obj = new IpcUint32(s);
			checkValue(obj, (int)l);
		}

		// Invalid String.
		try {
			new IpcUint32(null);
			needException();
		}
		catch (NullPointerException e) {}

		String[] invalid = {
			// no length.
			"",
			// out of range for each expression.
			"4294967296",
			"-1",
			"-12",
			"040000000000",
			"-01",
			"0x100000000",
			"-0x1",
			// miss expressions.
			"09876543",
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
				new IpcUint32(invalid[idx]);
				needException(invalid[idx]);
			}
			catch (NumberFormatException e) {}
		}
	}

	/**
	 * Test case for {@link IpcUint32#compareTo(IpcUint32)}.
	 */
	public void testCompareTo()
	{
		for (int loop = 0; loop < 10000; loop++) {
			long i = getTestValueLong(loop);
			long j = getTestValueLong(loop);
			IpcUint32 obji = new IpcUint32((int)i);
			IpcUint32 objj = new IpcUint32((int)j);
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
	 * Test case for {@link IpcUint32#equals(Object)} and
	 * {@link IpcUint32#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<IpcUint32> set = new HashSet<IpcUint32>();
		HashSet<Long> lset = new HashSet<Long>();

		for (int loop = 0; loop < 10000; loop++) {
			long l = getTestValueLong(loop);
			IpcUint32 obj = new IpcUint32(l);
			assertTrue(obj.equals(obj));
			assertFalse(obj.equals(null));
			assertFalse(obj.equals(new Object()));

			Long lobj = new Long(l);
			if (lset.add(lobj)) {
				assertTrue(set.add(obj));
				assertFalse(set.add(obj));
			}

			obj = new IpcUint32(l);
			assertFalse(set.add(obj));
		}
	}

	/**
	 * Ensure that the specified object has the specified value.
	 *
	 * @param obj		An object to be tested.
	 * @param value		Required value
	 */
	private void checkValue(IpcUint32 obj, int value)
	{
		long l = (long)value & 0xffffffffL;
		assertEquals((byte)l, obj.byteValue());
		assertEquals((short)l, obj.shortValue());
		assertEquals(value, obj.intValue());
		assertEquals(l, obj.longValue());
		assertEquals((float)l, obj.floatValue());
		assertEquals((double)l, obj.doubleValue());

		assertEquals(Long.toString(l), obj.toString());

		assertEquals(IpcDataUnit.UINT32, obj.getType());
	}

	/**
	 * <p>
	 * Detect value for each test with int variables. 
	 * </p>
	 * <p>
	 *   This method uses for detcting input values for each unit test.
	 *
	 * @param	idx	index of using values.
	 * @return	return index-th boundary value when index is in 
	 *		the dictionary of doundary values. 
	 *		othewise, return a randam number.
	 */
	private int getTestValueInt(int idx)
	{
		if (idx < boundary_int_values.length) {
			return boundary_int_values[idx];
		} else {
			return rand.nextInt();
		}
	}

	/**
	 * <p>
	 * Detect value for each test with long variables. 
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
	private long getTestValueLong(int idx)
	{
		if (idx < boundary_long_values.length) {
			return boundary_long_values[idx];
		} else {
			return rand.nextLong() & 0xffffffffL;
		}
	}
}
