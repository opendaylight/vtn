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
import org.opendaylight.vtn.core.util.UnsignedInteger;

/**
 * <p>
 *   Unit test class for {@link IpcUint64}.
 * </p>
 */
public class IpcUint64Test extends TestBase
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
	 * Create JUnit test case for {@link IpcUint64}.
	 *
	 * @param name	The test name.
	 */
	public IpcUint64Test(String name)
	{
		super(name);
		rand = new Random();
	}

	/**
	 * Test case for {@link IpcUint64#IpcUint64(long)}.
	 */
	public void testCtorLong()
	{
		for (int loop = 0; loop < 10000; loop++) {
			long l = getTestValue(loop);
			IpcUint64 obj = new IpcUint64(l);
			checkValue(obj, l);
		}
	}

	/**
	 * Test case for {@link IpcUint64#IpcUint64(String)}.
	 */
	public void testCtorString()
	{
		for (int loop = 0; loop < 10000; loop++) {
			long l = getTestValue(loop);

			// Decimal
			String str = UnsignedInteger.toString(l);
			IpcUint64 obj = new IpcUint64(l);
			checkValue(obj, l);

			// Octal
			str = "0" + UnsignedInteger.toOctalString(l);
			obj = new IpcUint64(str);
			checkValue(obj, l);

			// Hexadecimal
			String hex = UnsignedInteger.toHexString(l);
			str = "0x" + hex;
			obj = new IpcUint64(str);
			checkValue(obj, l);

			str = str.toUpperCase();
			obj = new IpcUint64(str);
			checkValue(obj, l);

			str = "#" + hex;
			obj = new IpcUint64(str);
			checkValue(obj, l);
		}

		// Invalid String.
		try {
			new IpcUint64(null);
			needException();
		}
		catch (NullPointerException e) {}

		String[] invalid = {
			// no length.
			"",
			// out of range for each expression.
			"18446744073709551616",
			"-1",
			"02000000000000000000000",
			"-01",
			"0x10000000000000000",
			"-0x1",
			// miss expressions.
			"0987654321",
			"0xX",
			"x1",
			// normal string
			"string",
			// type suffix expression. (not accept)
			"127L",
			"127 L",
		};

		for (int idx = 0; idx < invalid.length; idx++) {
			try {
				new IpcUint64(invalid[idx]);
				needException(invalid[idx]);
			}
			catch (NumberFormatException e) {}
		}
	}

	/**
	 * Test case for {@link IpcUint64#compareTo(IpcUint64)}.
	 */
	public void testCompareTo()
	{
		for (int loop = 0; loop < 10000; loop++) {
			long i = getTestValue(loop % 0x700);
			long j = getTestValue(loop % 0x1000);

			IpcUint64 obji = new IpcUint64(i);
			IpcUint64 objj = new IpcUint64(j);
			assertEquals(0, obji.compareTo(obji));
			assertEquals(0, objj.compareTo(objj));

			int answer = getCompareAnswer(i, j);
			int result = obji.compareTo(objj);
			if (answer == 0) {
				assertEquals(0, result);
			}
			else if (answer < 0) {
				assertTrue(result < 0);
			}
			else {
				assertTrue(result > 0);
			}			
			assertEquals(UnsignedInteger.compare(i, j),
				     obji.compareTo(objj));
		}
	}

	/**
	 * Test case for {@link IpcUint64#equals(Object)} and
	 * {@link IpcUint64#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<IpcUint64> set = new HashSet<IpcUint64>();
		HashSet<Long> lset = new HashSet<Long>();

		for (int loop = 0; loop < 10000; loop++) {
			long l = getTestValue(loop);
			IpcUint64 obj = new IpcUint64(l);
			assertTrue(obj.equals(obj));
			assertFalse(obj.equals(null));
			assertFalse(obj.equals(new Object()));

			Long lobj = new Long(l);
			if (lset.add(lobj)) {
				assertTrue(set.add(obj));
				assertFalse(set.add(obj));
			}

			obj = new IpcUint64(l);
			assertFalse(set.add(obj));
		}
	}

	/**
	 * Ensure that the specified object has the specified value.
	 *
	 * @param obj		An object to be tested.
	 * @param value		Required value
	 */
	private void checkValue(IpcUint64 obj, long value)
	{
		assertEquals((byte)value, obj.byteValue());
		assertEquals((short)value, obj.shortValue());
		assertEquals((int)value, obj.intValue());
		assertEquals(value, obj.longValue());
		assertEquals(UnsignedInteger.floatValue(value),
			     obj.floatValue());
		assertEquals(UnsignedInteger.doubleValue(value),
			     obj.doubleValue());

		assertEquals(UnsignedInteger.toString(value), obj.toString());

		assertEquals(IpcDataUnit.UINT64, obj.getType());
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


	/**
	 * <p>
	 * Creatte answer for compare function. 
	 * </p>
	 * <p>
	 *   This method uses for creating answer of compare test
	 *   without UnsignedInteger wrapper.
	 * </p>
	 *
	 * @param	li	1st compared value. This is base value.
	 * @param	lj	2nd compared value.
	 * @return	A negative integer(-1), zero, or a positive integer(1)
	 *		os returned if 1st value is less than, equal to,
	 *		or greater than the 2nd valuerespectively.
	 */
	private int getCompareAnswer(long li, long lj)
	{
		if (li == lj) {
			// answer is zero.
			return 0;
		} else  if (lj == 0) {
			// answer is plus.
			return 1;
		} else if  (li == 0) {
			// answer is minus.
			return -1;
		} else if (((li > 0) && (lj > 0)) ||
			   ((li < 0) && (lj < 0))) {
			// answer is equal to compare for signed.
			Long lo = new Long(li);
			return lo.compareTo(lj);
		} else {
			// answer is reverse for comparing signed number.
			Long lo = new Long(li);
			if ( lo.compareTo(lj) > 0) {
				return -1;
			} else {
				return 1;
			}
		}
	}
}
