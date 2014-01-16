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
 *   Unit test class for {@link IpcFloat}.
 * </p>
 */
public class IpcFloatTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link IpcFloat}.
	 *
	 * @param name	The test name.
	 */
	public IpcFloatTest(String name)
	{
		super(name);
	}

	/**
	 * Test case for {@link IpcFloat#IpcFloat(float)}.
	 */
	public void testCtorFloat()
	{
		float[] floats = {
			Float.POSITIVE_INFINITY,
			Float.NEGATIVE_INFINITY,
			Float.NaN,
			Float.MAX_VALUE,
			Float.MIN_NORMAL,
			Float.MIN_VALUE,
			0f,
			-0f,
			1.0f,
			1.2345f,
			100.45678f,
			-300.9999f,
			3.141592f,
			// un-default NaN
			Float.intBitsToFloat(0x7fc00001),
			Float.intBitsToFloat(0x7f800001),
			Float.intBitsToFloat(0x7fffffff),
			Float.intBitsToFloat(0xffc00001),
			Float.intBitsToFloat(0xff800001),
			Float.intBitsToFloat(0xffffffff),
		};

		for (int idx = 0; idx < floats.length; idx++) {
			float f = floats[idx];
			IpcFloat obj = new IpcFloat(f);
			checkValue(obj, f);
		}
	}

	/**
	 * Test case for {@link IpcFloat#IpcFloat(double)}.
	 */
	public void testCtorDouble()
	{
		double[] doubles = {
			Double.POSITIVE_INFINITY,
			Double.NEGATIVE_INFINITY,
			Double.NaN,
			Double.MAX_VALUE,
			Double.MIN_NORMAL,
			Double.MIN_VALUE,
			0.0,
			-0.0,
			1.0,
			1.2345,
			100.45678,
			-300.9999,
			Math.PI,
			Math.E,
			// un-default NaN for float
			Float.intBitsToFloat(0x7fc00001),
			Double.longBitsToDouble(0x7fc00001),
			Float.intBitsToFloat(0x7fc00000),
			Double.longBitsToDouble(0x7fc00000),
			Float.intBitsToFloat(0x7f7fffff),
			Double.longBitsToDouble(0x7f7fffff),
			Float.intBitsToFloat(0x7f800000),
			Double.longBitsToDouble(0x7f800000),
			Float.intBitsToFloat(0x7f800001),
			Double.longBitsToDouble(0x7f80000),
		};

		for (int idx = 0; idx < doubles.length; idx++) {
			double d = doubles[idx];
			IpcFloat obj = new IpcFloat(d);
			checkValue(obj, (float)d);			
		}
	}

	/**
	 * Test case for {@link IpcFloat#IpcFloat(String)}.
	 */
	public void testCtorString()
	{
		String[] strings = {
			// NaN expressions.
			"NaN",
			"+NaN",
			"-NaN",
			// Infinity expressions.
			"Infinity",
			"+Infinity",
			"-Infinity",
			// zero check.
			"0",
			"00",
			"+0",
			"-0",
			// normal expressions(omitting some parts).
			"-1",
			"1.",
			".1234",
			// Use zero at first number charactor.
			"0.1234",
			"00.1234",
			"01.1234",
			"-001.1234",
			// normal expressions.
			"3.141592",
			"100.123",
			"1024.000001",
			// exponent expressions.
			"3.8e3",
			"3.8e+2",
			"3.8e-2",
			// float suffix expressions.
			"3.8f",
			"+3.8e2f",
			"-3.8e2f",
			"3.8d",
			"-3.8e2d",
			// binary exponent expressions.
 			"0xap1",
 			"0Xa.p1",
 			"-0xa.bp-1",
			// round numbers
			"1024.000002",
			"1.00000017881393421514957253748434595763683319091796875001d",
			// huge numbers
			"1.0E+200000f",
			"1.0E-200000f",
		};
		float[] required = {
			// NaN expressions.
			Float.NaN,
			Float.NaN,
			Float.NaN,
			// Infinity expressions.
			Float.POSITIVE_INFINITY,
			Float.POSITIVE_INFINITY,
			Float.NEGATIVE_INFINITY,
			// zero check.
			0f,
			0f,
			0f,
			-0f,
			// normal expressions(omitting some parts).
			-1.0f,
			1.0f,
			0.1234f,
			// Use zero at first number charactor.
			0.1234f,
			0.1234f,
			1.1234f,
			-1.1234f,
			// normal expressions.
			3.141592f,
			100.123f,
			1024.000001f,
			// exponent expressions.
			3800.0f,
			380.0f,
			0.038f,
			// float suffix expressions.
			3.8f,
			380.0f,
			-380.0f,
			3.8f,
			-380.0f,
			// binary exponent expressions.
			20.0f,
			20.0f,
			-5.34375f,
			// round numbers
			1024.0f,
			1.0000001f,
			// huge numbers
			Float.POSITIVE_INFINITY,
			0f,
		};

		for (int idx = 0; idx < strings.length; idx++) {
			IpcFloat obj = new IpcFloat(strings[idx]);
			checkValue(obj, required[idx]);
		}

		// Invalid String.
		try {
			new IpcFloat(null);
			needException();
		}
		catch (NullPointerException e) {}

		String[] invalid = {
			// no length.
			"",
			// decimal expressions. (unsupport in float number).
			"0xff",
			"10L",
			// string
			"0xg",
			"string",
			// miss expression
			"1.2.3.4",
			// Java 7 expressions
			"101_010_101",
			// miss exponent expresions.
			"3.8p+2",
			"3.6p-2",
			"0xaE+2",
			"0xaE-2",
		};

		for (int idx = 0; idx < invalid.length; idx++) {
			try {
				new IpcFloat(invalid[idx]);
				needException(invalid[idx]);
			}
			catch (NumberFormatException e) {}
		}
	}

	/**
	 * Test case for {@link IpcFloat#compareTo(IpcFloat)}.
	 */
	public void testCompareTo()
	{
		float[][] tests = {
			// [0] = lesser, [1] == greater
			new float[]{-0f, 0f},
			new float[]{Float.NEGATIVE_INFINITY,
				    Float.POSITIVE_INFINITY},
			new float[]{Float.NEGATIVE_INFINITY, -10000000f},
			new float[]{1000000f, Float.POSITIVE_INFINITY},
			new float[]{Float.NEGATIVE_INFINITY, Float.NaN},
			new float[]{1f, Float.NaN},
			new float[]{Float.POSITIVE_INFINITY, Float.NaN},
			new float[]{0f, 0.0011f},
			new float[]{0.001f, 0.0011f},
			new float[]{-10.34567f, -10.345669f},
			new float[]{-1.00001f, 0f},
			new float[]{-1.5f, 10f},
		};
		for (int idx = 0; idx < tests.length; idx++) {
			float lesser = tests[idx][0];
			float greater = tests[idx][1];

			IpcFloat lobj = new IpcFloat(lesser);
			IpcFloat gobj = new IpcFloat(greater);
			assertEquals(0, lobj.compareTo(lobj));
			assertEquals(true, lobj.equals(lobj));
			assertEquals(0, gobj.compareTo(gobj));
			assertEquals(true, gobj.equals(gobj));
			assertTrue(lobj.compareTo(gobj) < 0);
			assertTrue(gobj.compareTo(lobj) > 0);

			IpcFloat lobj2 = new IpcFloat(lesser);	
			IpcFloat gobj2 = new IpcFloat(greater);
			assertEquals(0, lobj.compareTo(lobj2));
			assertEquals(true, lobj.equals(lobj2));
			assertEquals(0, gobj.compareTo(gobj2));
			assertEquals(true, gobj.equals(gobj2));			
		}

		// NaN tests
		float[][] tests_NaN = {
			// [0] = valuie 1 (NaN), [1] == value 2 (NaN)
			// Each value set has same weight.
			new float[]{Float.NaN, Float.NaN},
			new float[]{Float.NaN, 
				    Float.intBitsToFloat(0x7fc00001)},
			new float[]{Float.NaN, 
				    Float.intBitsToFloat(0x7f800001)},
			new float[]{Float.NaN, 
				    Float.intBitsToFloat(0x7fffffff)},
			new float[]{Float.NaN, 
				    Float.intBitsToFloat(0xffc00001)},
			new float[]{Float.NaN, 
				    Float.intBitsToFloat(0xff800001)},
			new float[]{Float.NaN, 
				    Float.intBitsToFloat(0xffffffff)},
			new float[]{Float.intBitsToFloat(0x7fc00001),
				    Float.intBitsToFloat(0x7fffffff)},
			new float[]{Float.intBitsToFloat(0x7fc00001),
				    Float.intBitsToFloat(0xffffffff)},
			new float[]{Float.intBitsToFloat(0xffc00001),
				    Float.intBitsToFloat(0x7fffffff)},
			new float[]{Float.intBitsToFloat(0xffc00001),
				    Float.intBitsToFloat(0xffffffff)},
		};
		for (int idx = 0; idx < tests_NaN.length; idx++) {
			float nanValue1 = tests_NaN[idx][0];
			float nanValue2 = tests_NaN[idx][1];

			IpcFloat obj1 = new IpcFloat(nanValue1);
			IpcFloat obj2 = new IpcFloat(nanValue2);
			assertEquals(0, obj1.compareTo(obj1));
			assertEquals(true, obj1.equals(obj1));
			assertEquals(0, obj2.compareTo(obj2));
			assertEquals(true, obj2.equals(obj2));
			assertEquals(0, obj1.compareTo(obj2));
			assertEquals(0, obj2.compareTo(obj1));
			assertEquals(true, obj1.equals(obj2));
			assertEquals(true, obj2.equals(obj1));
			assertTrue(obj1.floatValue() != obj2.floatValue());
		}
	}

	/**
	 * Test case for {@link IpcFloat#equals(Object)} and
	 * {@link IpcFloat#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<IpcFloat> set = new HashSet<IpcFloat>();
		HashSet<Float> fset = new HashSet<Float>();
		Random rand = new Random();
		String randtype =
			System.getProperty("pflow.core.ipc.test.randtype");

		for (int loop = 0; loop < 0x10000; loop++) {
			float f;
			if ((randtype != null) && randtype.equals("exchange")) {
				f = Float.intBitsToFloat(rand.nextInt());
			} else {
				f = (float)rand.nextLong();
			}
			IpcFloat obj = new IpcFloat(f);
			assertTrue(obj.equals(obj));
			assertFalse(obj.equals(null));
			assertFalse(obj.equals(new Object()));

			Float fobj = new Float(f);
			if (fset.add(fobj)) {
				assertTrue(set.add(obj));
				assertFalse(set.add(obj));
			}

			obj = new IpcFloat(f);
			assertFalse(set.add(obj));
		}
	}

	/**
	 * Ensure that the specified object has the specified value.
	 *
	 * @param obj		An object to be tested.
	 * @param value		Required value
	 */
	private void checkValue(IpcFloat obj, float value)
	{
		assertEquals((byte)value, obj.byteValue());
		assertEquals((short)value, obj.shortValue());
		assertEquals((int)value, obj.intValue());
		assertEquals((long)value, obj.longValue());
		assertEquals(value, obj.floatValue());
		assertEquals((double)value, obj.doubleValue());

		assertEquals(Float.toString(value), obj.toString());

		assertEquals(IpcDataUnit.FLOAT, obj.getType());
	}
}
