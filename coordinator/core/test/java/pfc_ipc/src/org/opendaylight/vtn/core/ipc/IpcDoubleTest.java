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
 *   Unit test class for {@link IpcDouble}.
 * </p>
 */
public class IpcDoubleTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link IpcDouble}.
	 *
	 * @param name	The test name.
	 */
	public IpcDoubleTest(String name)
	{
		super(name);
	}

	/**
	 * Test case for {@link IpcDouble#IpcDouble(float)}.
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
			IpcDouble obj = new IpcDouble(f);
			checkValue(obj, (double)f);
		}
	}

	/**
	 * Test case for {@link IpcDouble#IpcDouble(double)}.
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
			// un-default NaN
			Double.longBitsToDouble(0x7ff0000000000001L),
			Double.longBitsToDouble(0x7ff8000000000001L),
			Double.longBitsToDouble(0x7fffffffffffffffL),
			Double.longBitsToDouble(0xfff0000000000001L),
			Double.longBitsToDouble(0xfff8000000000001L),
			Double.longBitsToDouble(0xffffffffffffffffL),
		};

		for (int idx = 0; idx < doubles.length; idx++) {
			double d = doubles[idx];
			IpcDouble obj = new IpcDouble(d);
			checkValue(obj, d);
		}
	}

	/**
	 * Test case for {@link IpcDouble#IpcDouble(String)}.
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
			// check numbers about round.
			"1024.00000000000002",
			"1024.000002f",
			"0.10000000149011612",
			"1.00000017881393421514957253748434595763683319091796875001d",
			// huge numbers
			"1.0E+200000f",
			"1.0E-200000f",
		};
		double[] required = {
			// NaN expressions.
			Double.NaN,
			Double.NaN,
			Double.NaN,
			// Infinity expressions.
			Double.POSITIVE_INFINITY,
			Double.POSITIVE_INFINITY,
			Double.NEGATIVE_INFINITY,
			// zero check.
			0.0,
			0.0,
			0.0,
			-0.0,
			// normal expressions(omitting some parts).
			-1.0,
			1.0,
			0.1234,
			// Use zero at first number charactor.
			0.1234,
			0.1234,
			1.1234,
			-1.1234,
			// normal expressions.
			3.141592,
			100.123,
			1024.000001,
			// exponent expressions.
			3800.0,
			380.0,
			0.038,
			// float suffix expressions.
			3.8,
			380.0,
			-380.0,
			3.8,
			-380.0,
			// binary exponent expressions.
			20.0,
			20.0,
			-5.34375,
			// check numbers about round.
			1024.0,
			1024.000002,
			0.1f,
			1.00000017881393421514957253748434595763683319091796875001,
			// huge numbers
			Double.POSITIVE_INFINITY,
			0,
		};

		for (int idx = 0; idx < strings.length; idx++) {
			IpcDouble obj = new IpcDouble(strings[idx]);
			checkValue(obj, required[idx]);
		}

		// Invalid String.
		try {
			new IpcDouble(null);
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
			"0xaa_aa.p2",
			// miss exponent expresions.
			"3.8p+2",
			"3.6p-2",
			"0xaE+2",
			"0xaE-2",
		};

		for (int idx = 0; idx < invalid.length; idx++) {
			try {
				new IpcDouble(invalid[idx]);
				needException(invalid[idx]);
			}
			catch (NumberFormatException e) {}
		}
	}

	/**
	 * Test case for {@link IpcDouble#compareTo(IpcDouble)}.
	 */
	public void testCompareTo()
	{
		double[][] tests = {
			// [0] = lesser, [1] == greater
			new double[]{-0.0, 0.0},
			new double[]{Double.NEGATIVE_INFINITY,
				    Double.POSITIVE_INFINITY},
			new double[]{Double.NEGATIVE_INFINITY, -10000000.0},
			new double[]{1000000.0, Double.POSITIVE_INFINITY},
			new double[]{Double.NEGATIVE_INFINITY, Double.NaN},
			new double[]{1.0, Double.NaN},
			new double[]{Double.POSITIVE_INFINITY, Double.NaN},
			new double[]{0.0, 0.0011},
			new double[]{0.001, 0.0011},
			new double[]{-10.34567, -10.345669999},
			new double[]{-1.00001, 0},
			new double[]{-1.5, 10},
		};
		for (int idx = 0; idx < tests.length; idx++) {
			double lesser = tests[idx][0];
			double greater = tests[idx][1];

			IpcDouble lobj = new IpcDouble(lesser);
			IpcDouble gobj = new IpcDouble(greater);
			assertEquals(0, lobj.compareTo(lobj));
			assertEquals(true, lobj.equals(lobj));
			assertEquals(0, gobj.compareTo(gobj));
			assertEquals(true, gobj.equals(gobj));
			assertTrue(lobj.compareTo(gobj) < 0);
			assertTrue(gobj.compareTo(lobj) > 0);

			IpcDouble lobj2 = new IpcDouble(lesser);	
			IpcDouble gobj2 = new IpcDouble(greater);
			assertEquals(0, lobj.compareTo(lobj2));
			assertEquals(true, lobj.equals(lobj2));
			assertEquals(0, gobj.compareTo(gobj2));
			assertEquals(true, gobj.equals(gobj2));	
		}
		
		// NaN tests
		double[][] tests_NaN = {
			// [0] = valuie 1 (NaN), [1] == value 2 (NaN)
			// Each value set has same weight.
			new double[]{Double.NaN, Double.NaN},
			new double[]{Double.NaN, 
				    Double.longBitsToDouble(0x7ff8000000000001L)},
			new double[]{Double.NaN, 
				    Double.longBitsToDouble(0x7ff0000000000001L)},
			new double[]{Double.NaN, 
				    Double.longBitsToDouble(0x7fffffffffffffffL)},
			new double[]{Double.NaN, 
				    Double.longBitsToDouble(0xfff8000000000001L)},
			new double[]{Double.NaN, 
				    Double.longBitsToDouble(0xfff0000000000001L)},
			new double[]{Double.NaN, 
				    Double.longBitsToDouble(0xffffffffffffffffL)},
			new double[]{Double.longBitsToDouble(0x7ff8000000000001L),
				    Double.longBitsToDouble(0x7fffffffffffffffL)},
			new double[]{Double.longBitsToDouble(0x7ff8000000000001L),
				    Double.longBitsToDouble(0xffffffffffffffffL)},
			new double[]{Double.longBitsToDouble(0xfff8000000000001L),
				    Double.longBitsToDouble(0x7fffffffffffffffL)},
			new double[]{Double.longBitsToDouble(0xfff8000000000001L),
				    Double.longBitsToDouble(0xffffffffffffffffL)},
		};
		for (int idx = 0; idx < tests_NaN.length; idx++) {
			double nanValue1 = tests_NaN[idx][0];
			double nanValue2 = tests_NaN[idx][1];

			IpcDouble obj1 = new IpcDouble(nanValue1);
			IpcDouble obj2 = new IpcDouble(nanValue2);
			assertEquals(0, obj1.compareTo(obj1));
			assertEquals(true, obj1.equals(obj1));
			assertEquals(0, obj2.compareTo(obj2));
			assertEquals(true, obj2.equals(obj2));
			assertEquals(0, obj1.compareTo(obj2));
			assertEquals(0, obj2.compareTo(obj1));
			assertEquals(true, obj1.equals(obj2));
			assertEquals(true, obj2.equals(obj1));
			assertTrue(obj1.doubleValue() != obj2.doubleValue());
		}
	}

	/**
	 * Test case for {@link IpcDouble#equals(Object)} and
	 * {@link IpcDouble#hashCode()}.
	 */
	public void testEquals()
	{
		HashSet<IpcDouble> set = new HashSet<IpcDouble>();
		HashSet<Double> dset = new HashSet<Double>();
		Random rand = new Random();
		String randtype =
			System.getProperty("pflow.core.ipc.test.randtype");

		for (int loop = 0; loop < 0x10000; loop++) {
			double d;
			if ((randtype != null) && randtype.equals("exchange")) {
				d = Double.longBitsToDouble(rand.nextLong());
			} else {
				d = (double)rand.nextLong();
			}
			IpcDouble obj = new IpcDouble(d);
			assertTrue(obj.equals(obj));
			assertFalse(obj.equals(null));
			assertFalse(obj.equals(new Object()));

			Double dobj = new Double(d);
			if (dset.add(dobj)) {
				assertTrue(set.add(obj));
				assertFalse(set.add(obj));
			}

			obj = new IpcDouble(d);
			assertFalse(set.add(obj));
		}
	}

	/**
	 * Ensure that the specified object has the specified value.
	 *
	 * @param obj		An object to be tested.
	 * @param value		Required value
	 */
	private void checkValue(IpcDouble obj, double value)
	{
		assertEquals((byte)value, obj.byteValue());
		assertEquals((short)value, obj.shortValue());
		assertEquals((int)value, obj.intValue());
		assertEquals((long)value, obj.longValue());
		assertEquals((float)value, obj.floatValue());
		assertEquals(value, obj.doubleValue());

		assertEquals(Double.toString(value), obj.toString());

		assertEquals(IpcDataUnit.DOUBLE, obj.getType());
	}
}
