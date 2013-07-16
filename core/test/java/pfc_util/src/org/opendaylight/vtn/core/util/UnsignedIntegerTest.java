/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

import java.util.Random;
import java.util.LinkedList;
import java.util.Iterator;

import org.opendaylight.vtn.core.CoreSystem;

/**
 * <p>
 *   Unit test class for {@link UnsignedInteger}.
 * </p>
 */
public class UnsignedIntegerTest extends TestBase
{
	/**
	 * Create JUnit test case for {@link UnsignedInteger}.
	 *
	 * @param name	The test name.
	 */
	public UnsignedIntegerTest(String name)
	{
		super(name);
	}

	/**
	 * Test case for conversion to {@code short}.
	 */
	public void testShortValue()
	{
		// byte to short
		for (int i = Byte.MIN_VALUE; i <= Byte.MAX_VALUE; i++) {
			byte b = (byte)i;
			assertEquals((byte)(i & 0xff), b);
			short s = UnsignedInteger.shortValue(b);
			assertTrue(s >= 0);
			assertEquals((short)(i & 0xff), s);
		}
	}

	/**
	 * Test case for conversion to {@code int}.
	 */
	public void testIntValue()
	{
		// byte to int
		for (int i = Byte.MIN_VALUE; i <= Byte.MAX_VALUE; i++) {
			byte b = (byte)i;
			assertEquals((byte)(i & 0xff), b);
			int value = UnsignedInteger.intValue(b);
			assertTrue(value >= 0);
			assertEquals(i & 0xff, value);
		}

		// short to int
		for (int i = Short.MIN_VALUE; i <= Short.MAX_VALUE; i++) {
			short s = (short)i;
			assertEquals((short)(i & 0xffff), s);
			int value = UnsignedInteger.intValue(s);
			assertTrue(value >= 0);
			assertEquals(i & 0xffff, value);
		}
	}

	/**
	 * Test case for conversion to {@code long}.
	 */
	public void testLongValue()
	{
		// byte to long
		for (long i = Byte.MIN_VALUE; i <= Byte.MAX_VALUE; i++) {
			byte b = (byte)i;
			assertEquals((byte)(i & 0xff), b);
			long value = UnsignedInteger.longValue(b);
			assertTrue(value >= 0);
			assertEquals(i & 0xff, value);
		}

		// short to int
		for (long i = Short.MIN_VALUE; i <= Short.MAX_VALUE; i++) {
			short s = (short)i;
			assertEquals((short)(i & 0xffff), s);
			long value = UnsignedInteger.longValue(s);
			assertTrue(value >= 0);
			assertEquals(i & 0xffff, value);
		}

		// int to long
		long values[] = {
			Integer.MIN_VALUE,
			Integer.MIN_VALUE + 1,
			Integer.MIN_VALUE + 2,
			Integer.MIN_VALUE + 3,
			-5, -4, -3, -2, -1,
			0, 1, 2, 3, 4, 5,
			Integer.MAX_VALUE - 3,
			Integer.MAX_VALUE - 2,
			Integer.MAX_VALUE - 1,
			Integer.MAX_VALUE,
		};
		for (int idx = 0; idx < values.length; idx++) {
			long l = values[idx];
			int  i = (int)l;
			assertEquals((int)(l & 0xffffffffL), i);
			long value = UnsignedInteger.longValue(i);
			assertTrue(value >= 0);
			assertEquals(l & 0xffffffffL, value);
		}

		Random rand = new Random();
		for (int loop = 0; loop < 10000; loop++) {
			long l = rand.nextLong();
			int  i = (int)l;
			assertEquals((int)(l & 0xffffffffL), i);
			long value = UnsignedInteger.longValue(i);
			assertTrue(value >= 0);
			assertEquals(l & 0xffffffffL, value);
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#floatValue(byte)}.
	 */
	public void testFloatValueByte()
	{
		for (int i = 0; i <= 0xff; i++) {
			byte b = (byte)i;
			float f = UnsignedInteger.floatValue(b);
			assertEquals((float)i, f);
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#floatValue(short)}.
	 */
	public void testFloatValueShort()
	{
		for (int i = 0; i <= 0xffff; i++) {
			short s = (short)i;
			float f = UnsignedInteger.floatValue(s);
			assertEquals((float)i, f);
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#floatValue(int)}.
	 */
	public void testFloatValueInt()
	{
		LinkedList<Long> values = new LinkedList<Long>();
		values.add(new Long(0));
		values.add(new Long(1));
		values.add(new Long(2));
		values.add(new Long(3));
		values.add(new Long(4));
		values.add(new Long(5));
		values.add(new Long(0x1000L));
		values.add(new Long(0x1001L));
		values.add(new Long(0x1002L));
		values.add(new Long(0x1003L));
		values.add(new Long(0x7ffffffeL));
		values.add(new Long(0x7fffffffL));
		values.add(new Long(0x80000000L));
		values.add(new Long(0x80000001L));
		values.add(new Long(0x80000002L));
		values.add(new Long(0xfffffffdL));
		values.add(new Long(0xfffffffeL));
		values.add(new Long(0xffffffffL));

		Random rand = new Random();
		for (int loop = 0; loop < 10000; loop++) {
			int i = rand.nextInt();
			values.add(new Long(UnsignedInteger.longValue(i)));
		}
		
		for (Iterator<Long> it = values.iterator(); it.hasNext();) {
			Long lg = it.next();
			int  i = lg.intValue();
			float f = UnsignedInteger.floatValue(i);
			assertEquals(lg.floatValue(), f);
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#floatValue(long)}.
	 */
	public void testFloatValueLong()
	{
		float[] required = {
			0f, 1f, 2f, 3f, 4f, 5f, 6f, 7f, 8f, 9f, 10f,
			11f, 12f, 13f, 14f, 15f, 16f,
			268435455f,
			268435456f,
			268435457f,
			2147483646f,
			2147483647f,
			2147483648f,
			2147483649f,
			2147483650f,
			4294967293f,
			4294967294f,
			4294967295f,
			4294967296f,
			4294967297f,
			4294967298f,
			4294967299f,
			9223372036854775806f,
			9223372036854775807f,
			9223372036854775808f,
			9223372036854775809f,
			9223372036854775810f,
			17293822569102704639f,
			17293822569102704640f,
			17293822569102704641f,
			18446744073709551613f,
			18446744073709551614f,
			18446744073709551615f,
		};
		long[] values = {
			0x0L, 0x1L, 0x2L, 0x3L, 0x4L, 0x5L, 0x6L, 0x7L,
			0x8L, 0x9L, 0xaL, 0xbL, 0xcL, 0xdL, 0xeL, 0xfL,
			0x10L,
			0xfffffffL,
			0x10000000L,
			0x10000001L,
			0x7ffffffeL,
			0x7fffffffL,
			0x80000000L,
			0x80000001L,
			0x80000002L,
			0xfffffffdL,
			0xfffffffeL,
			0xffffffffL,
			0x100000000L,
			0x100000001L,
			0x100000002L,
			0x100000003L,
			0x7ffffffffffffffeL,
			0x7fffffffffffffffL,
			-9223372036854775808L,		// 0x8000000000000000
			-9223372036854775807L,		// 0x8000000000000001
			-9223372036854775806L,		// 0x8000000000000002
			-1152921504606846977L,		// 0xefffffffffffffff
			-1152921504606846976L,		// 0xf000000000000000
			-1152921504606846975L,		// 0xf000000000000001
			-3L,				// 0xfffffffffffffffd
			-2L,				// 0xfffffffffffffffe
			-1L,				// 0xffffffffffffffff
		};

		for (int idx = 0; idx < required.length; idx++) {
			float f = UnsignedInteger.floatValue(values[idx]);
			assertEquals(required[idx], f);
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#doubleValue(byte)}.
	 */
	public void testDoubleValueByte()
	{
		for (int i = 0; i <= 0xff; i++) {
			byte b = (byte)i;
			double d = UnsignedInteger.doubleValue(b);
			assertEquals((double)i, d);
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#doubleValue(short)}.
	 */
	public void testDoubleValueShort()
	{
		for (int i = 0; i <= 0xffff; i++) {
			short s = (short)i;
			double d = UnsignedInteger.doubleValue(s);
			assertEquals((double)i, d);
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#doubleValue(int)}.
	 */
	public void testDoubleValueInt()
	{
		LinkedList<Long> values = createIntTestData();

		for (Iterator<Long> it = values.iterator(); it.hasNext();) {
			Long lg = it.next();
			int  i = lg.intValue();
			double d = UnsignedInteger.doubleValue(i);
			assertEquals(lg.doubleValue(), d);
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#doubleValue(long)}.
	 */
	public void testDoubleValueLong()
	{
		double[] required = {
			0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0,
			11.0, 12.0, 13.0, 14.0, 15.0, 16.0,
			268435455.0,
			268435456.0,
			268435457.0,
			2147483646.0,
			2147483647.0,
			2147483648.0,
			2147483649.0,
			2147483650.0,
			4294967293.0,
			4294967294.0,
			4294967295.0,
			4294967296.0,
			4294967297.0,
			4294967298.0,
			4294967299.0,
			9223372036854775806.0,
			9223372036854775807.0,
			9223372036854775808.0,
			9223372036854775809.0,
			9223372036854775810.0,
			17293822569102704639.0,
			17293822569102704640.0,
			17293822569102704641.0,
			18446744073709551613.0,
			18446744073709551614.0,
			18446744073709551615.0,
		};
		long[] values = {
			0x0L, 0x1L, 0x2L, 0x3L, 0x4L, 0x5L, 0x6L, 0x7L,
			0x8L, 0x9L, 0xaL, 0xbL, 0xcL, 0xdL, 0xeL, 0xfL,
			0x10L,
			0xfffffffL,
			0x10000000L,
			0x10000001L,
			0x7ffffffeL,
			0x7fffffffL,
			0x80000000L,
			0x80000001L,
			0x80000002L,
			0xfffffffdL,
			0xfffffffeL,
			0xffffffffL,
			0x100000000L,
			0x100000001L,
			0x100000002L,
			0x100000003L,
			0x7ffffffffffffffeL,
			0x7fffffffffffffffL,
			-9223372036854775808L,		// 0x8000000000000000
			-9223372036854775807L,		// 0x8000000000000001
			-9223372036854775806L,		// 0x8000000000000002
			-1152921504606846977L,		// 0xefffffffffffffff
			-1152921504606846976L,		// 0xf000000000000000
			-1152921504606846975L,		// 0xf000000000000001
			-3L,				// 0xfffffffffffffffd
			-2L,				// 0xfffffffffffffffe
			-1L,				// 0xffffffffffffffff
		};

		for (int idx = 0; idx < required.length; idx++) {
			double d = UnsignedInteger.doubleValue(values[idx]);
			assertEquals(required[idx], d);
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#parseByte(String)}.
	 */
	public void testParseByte()
	{
		for (int i = 0; i <= 0xff; i++) {
			// Decimal
			String str = Integer.toString(i);
			byte value = UnsignedInteger.parseByte(str);
			assertEquals((byte)i, value);
			value = UnsignedInteger.parseByte("+" + str);
			assertEquals((byte)i, value);

			// Octal
			str = "0" + Integer.toOctalString(i);
			value = UnsignedInteger.parseByte(str);
			assertEquals((byte)i, value);
			value = UnsignedInteger.parseByte("+" + str);
			assertEquals((byte)i, value);

			// Hexadecimal
			String hex = Integer.toHexString(i);
			str = "0x" + hex;
			value = UnsignedInteger.parseByte(str);
			assertEquals((byte)i, value);
			value = UnsignedInteger.parseByte("+" + str);
			assertEquals((byte)i, value);

			str = str.toUpperCase();
			value = UnsignedInteger.parseByte(str);
			assertEquals((byte)i, value);
			value = UnsignedInteger.parseByte("+" + str);
			assertEquals((byte)i, value);

			str = "#" + hex;
			value = UnsignedInteger.parseByte(str);
			assertEquals((byte)i, value);
			value = UnsignedInteger.parseByte("+" + str);
			assertEquals((byte)i, value);
		}

		// Specifying null.
		try {
			UnsignedInteger.parseByte(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class, null);
		}

		// Invalid strings.
		String[] strings = {
			// Empty strings.
			"",
			"+",
			"-",
			"#",
			"+#",
			"-#",
			"0x",
			"+0x",
			"-0x",

			// Negative strings.
			"-1",
			"-2",
			"-3",
			"-01",
			"-02",
			"-03",
			"-0x10",
			"-0x11",
			"-0x12",

			// Invalid character.
			"1f",
			"078",
			"0xag",
			" 0xff",
			"120 ",
		};
		for (int idx = 0; idx < strings.length; idx++) {
			try {
				UnsignedInteger.parseByte(strings[idx]);
				needException();
			}
			catch (Exception e) {
				checkException(e, NumberFormatException.class,
					       "Invalid string: " +
					       strings[idx]);
			}
		}

		// Too large values.
		strings = new String[] {
			"0x100",
			"0x101",
			"0x102",
			"0x103",
			"0x80000000",
			"0x80000001",
			"0xfffffffe",
			"0xffffffff",
			"0x100000000",
			"0x100000001",
			"0x100000002",
			"0x100000003",
			"0x10000000000000000",
			"0x10000000000000001",
			"0x10000000000000002",
			"0x10000000000000003",
		};
		for (int idx = 0; idx < strings.length; idx++) {
			try {
				UnsignedInteger.parseByte(strings[idx]);
				needException();
			}
			catch (Exception e) {
				checkException(e, NumberFormatException.class,
					       "Out of range: " +
					       strings[idx]);
			}
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#parseShort(String)}.
	 */
	public void testParseShort()
	{
		for (int i = 0; i <= 0xffff; i += 3) {
			parseShortTest(i);
		}
		parseShortTest(0xffff);

		// Specifying null.
		try {
			UnsignedInteger.parseShort(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class, null);
		}

		// Invalid strings.
		String[] strings = {
			// Empty strings.
			"",
			"+",
			"-",
			"#",
			"+#",
			"-#",
			"0x",
			"+0x",
			"-0x",

			// Negative strings.
			"-1",
			"-2",
			"-3",
			"-01",
			"-02",
			"-03",
			"-0x10",
			"-0x11",
			"-0x12",

			// Invalid character.
			"1f",
			"078",
			"0xag",
			" 0xff",
			"120 ",
		};
		for (int idx = 0; idx < strings.length; idx++) {
			try {
				UnsignedInteger.parseShort(strings[idx]);
				needException();
			}
			catch (Exception e) {
				checkException(e, NumberFormatException.class,
					       "Invalid string: " +
					       strings[idx]);
			}
		}

		// Too large values.
		strings = new String[] {
			"0x10000",
			"0x10001",
			"0x10002",
			"0x10003",
			"0x80000000",
			"0x80000001",
			"0xfffffffe",
			"0xffffffff",
			"0x100000000",
			"0x100000001",
			"0x100000002",
			"0x100000003",
			"0x10000000000000000",
			"0x10000000000000001",
			"0x10000000000000002",
			"0x10000000000000003",
		};
		for (int idx = 0; idx < strings.length; idx++) {
			try {
				UnsignedInteger.parseShort(strings[idx]);
				needException();
			}
			catch (Exception e) {
				checkException(e, NumberFormatException.class,
					       "Out of range: " +
					       strings[idx]);
			}
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#parseInt(String)}.
	 */
	public void testParseInt()
	{
		LinkedList<Long> values = createIntTestData();
		
		for (Iterator<Long> it = values.iterator(); it.hasNext();) {
			Long lg = it.next();
			long l = lg.longValue();
			int  i = lg.intValue();

			// Decimal
			String str = lg.toString();
			int value = UnsignedInteger.parseInt(str);
			assertEquals(i, value);
			value = UnsignedInteger.parseInt("+" + str);
			assertEquals(i, value);

			// Octal
			str = "0" + Long.toOctalString(l);
			value = UnsignedInteger.parseInt(str);
			assertEquals(i, value);
			value = UnsignedInteger.parseInt("+" + str);
			assertEquals(i, value);

			// Hexadecimal
			String hex = Long.toHexString(l);
			str = "0x" + hex;
			value = UnsignedInteger.parseInt(str);
			assertEquals(i, value);
			value = UnsignedInteger.parseInt("+" + str);
			assertEquals(i, value);

			str = str.toUpperCase();
			value = UnsignedInteger.parseInt(str);
			assertEquals(i, value);
			value = UnsignedInteger.parseInt("+" + str);
			assertEquals(i, value);

			str = "#" + hex;
			value = UnsignedInteger.parseInt(str);
			assertEquals(i, value);
			value = UnsignedInteger.parseInt("+" + str);
			assertEquals(i, value);
		}

		// Specifying null.
		try {
			UnsignedInteger.parseInt(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class, null);
		}

		// Invalid strings.
		String[] strings = {
			// Empty strings.
			"",
			"+",
			"-",
			"#",
			"+#",
			"-#",
			"0x",
			"+0x",
			"-0x",

			// Negative strings.
			"-1",
			"-2",
			"-3",
			"-01",
			"-02",
			"-03",
			"-0x10",
			"-0x11",
			"-0x12",

			// Invalid character.
			"1f",
			"078",
			"0xag",
			" 0xff",
			"120 ",
		};
		for (int idx = 0; idx < strings.length; idx++) {
			try {
				UnsignedInteger.parseInt(strings[idx]);
				needException();
			}
			catch (Exception e) {
				checkException(e, NumberFormatException.class,
					       "Invalid string: " +
					       strings[idx]);
			}
		}

		// Too large values.
		strings = new String[] {
			"0x100000000",
			"0x100000001",
			"0x100000002",
			"0x100000003",
			"0x10000000000000000",
			"0x10000000000000001",
			"0x10000000000000002",
			"0x10000000000000003",
		};
		for (int idx = 0; idx < strings.length; idx++) {
			try {
				UnsignedInteger.parseInt(strings[idx]);
				needException();
			}
			catch (Exception e) {
				checkException(e, NumberFormatException.class,
					       "Out of range: " +
					       strings[idx]);
			}
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#parseLong(String)}.
	 */
	public void testParseLong()
	{
		String[] decimal = {
			"0", "1", "2", "3", "4", "5", "6", "7",
			"8", "9", "10", "11", "12", "13", "14", "15", "16",
			"268435455",
			"268435456",
			"268435457",
			"2147483646",
			"2147483647",
			"2147483648",
			"2147483649",
			"2147483650",
			"4294967293",
			"4294967294",
			"4294967295",
			"4294967296",
			"4294967297",
			"4294967298",
			"4294967299",
			"9223372036854775806",
			"9223372036854775807",
			"9223372036854775808",
			"9223372036854775809",
			"9223372036854775810",
			"17293822569102704639",
			"17293822569102704640",
			"17293822569102704641",
			"18446744073709551613",
			"18446744073709551614",
			"18446744073709551615",
		};
		String[] octal = {
			"00", "01", "02", "03", "04", "05", "06", "07",
			"010", "011", "012", "013", "014", "015", "016", "017",
			"020",
			"01777777777",
			"02000000000",
			"02000000001",
			"017777777776",
			"017777777777",
			"020000000000",
			"020000000001",
			"020000000002",
			"037777777775",
			"037777777776",
			"037777777777",
			"040000000000",
			"040000000001",
			"040000000002",
			"040000000003",
			"0777777777777777777776",
			"0777777777777777777777",
			"01000000000000000000000",
			"01000000000000000000001",
			"01000000000000000000002",
			"01677777777777777777777",
			"01700000000000000000000",
			"01700000000000000000001",
			"01777777777777777777775",
			"01777777777777777777776",
			"01777777777777777777777",
		};
		String[] hexadecimal = {
			"0", "1", "2", "3", "4", "5", "6", "7",
			"8", "9", "a", "b", "c", "d", "e", "f",
			"10",
			"fffffff",
			"10000000",
			"10000001",
			"7ffffffe",
			"7fffffff",
			"80000000",
			"80000001",
			"80000002",
			"fffffffd",
			"fffffffe",
			"ffffffff",
			"100000000",
			"100000001",
			"100000002",
			"100000003",
			"7ffffffffffffffe",
			"7fffffffffffffff",
			"8000000000000000",
			"8000000000000001",
			"8000000000000002",
			"efffffffffffffff",
			"f000000000000000",
			"f000000000000001",
			"fffffffffffffffd",
			"fffffffffffffffe",
			"ffffffffffffffff",
		};
		long[] required = {
			0x0L, 0x1L, 0x2L, 0x3L, 0x4L, 0x5L, 0x6L, 0x7L,
			0x8L, 0x9L, 0xaL, 0xbL, 0xcL, 0xdL, 0xeL, 0xfL,
			0x10L,
			0xfffffffL,
			0x10000000L,
			0x10000001L,
			0x7ffffffeL,
			0x7fffffffL,
			0x80000000L,
			0x80000001L,
			0x80000002L,
			0xfffffffdL,
			0xfffffffeL,
			0xffffffffL,
			0x100000000L,
			0x100000001L,
			0x100000002L,
			0x100000003L,
			0x7ffffffffffffffeL,
			0x7fffffffffffffffL,
			-9223372036854775808L,		// 0x8000000000000000
			-9223372036854775807L,		// 0x8000000000000001
			-9223372036854775806L,		// 0x8000000000000002
			-1152921504606846977L,		// 0xefffffffffffffff
			-1152921504606846976L,		// 0xf000000000000000
			-1152921504606846975L,		// 0xf000000000000001
			-3L,				// 0xfffffffffffffffd
			-2L,				// 0xfffffffffffffffe
			-1L,				// 0xffffffffffffffff
		};
		for (int idx = 0; idx < required.length; idx++) {
			// Decimal
			String str = decimal[idx];
			long req = required[idx];
			long value = UnsignedInteger.parseLong(str);
			assertEquals(req, value);
			value = UnsignedInteger.parseLong("+" + str);
			assertEquals(req, value);

			// Octal
			str = octal[idx];
			value = UnsignedInteger.parseLong(str);
			assertEquals(req, value);
			value = UnsignedInteger.parseLong("+" + str);
			assertEquals(req, value);

			// Hexadecimal
			String hex = hexadecimal[idx];
			str = "0x" + hex;
			value = UnsignedInteger.parseLong(str);
			assertEquals(req, value);
			value = UnsignedInteger.parseLong("+" + str);
			assertEquals(req, value);

			str = str.toUpperCase();
			value = UnsignedInteger.parseLong(str);
			assertEquals(req, value);
			value = UnsignedInteger.parseLong("+" + str);
			assertEquals(req, value);

			str = "#" + hex;
			value = UnsignedInteger.parseLong(str);
			assertEquals(req, value);
			value = UnsignedInteger.parseLong("+" + str);
			assertEquals(req, value);
		}

		// Specifying null.
		try {
			UnsignedInteger.parseLong(null);
			needException();
		}
		catch (Exception e) {
			checkException(e, NullPointerException.class, null);
		}

		// Invalid strings.
		String[] strings = {
			// Empty strings.
			"",
			"+",
			"-",
			"#",
			"+#",
			"-#",
			"0x",
			"+0x",
			"-0x",

			// Negative strings.
			"-1",
			"-2",
			"-3",
			"-01",
			"-02",
			"-03",
			"-0x10",
			"-0x11",
			"-0x12",

			// Invalid character.
			"1f",
			"078",
			"0xag",
			" 0xff",
			"120 ",
		};
		for (int idx = 0; idx < strings.length; idx++) {
			try {
				UnsignedInteger.parseLong(strings[idx]);
				needException();
			}
			catch (Exception e) {
				checkException(e, NumberFormatException.class,
					       "Invalid string: " +
					       strings[idx]);
			}
		}

		// Too large values.
		strings = new String[] {
			"0x10000000000000000",
			"0x10000000000000001",
			"0x10000000000000002",
			"0x10000000000000003",
			"0x1111111111111111111",
			"18446744073709551616",
			"18446744073709551617",
			"18446744073709551618",
			"111111111111111111111111",
			"02000000000000000000000",
			"02000000000000000000001",
			"02000000000000000000002",
			"00333333333333333333333333333333",
		};
		for (int idx = 0; idx < strings.length; idx++) {
			try {
				UnsignedInteger.parseLong(strings[idx]);
				needException();
			}
			catch (Exception e) {
				checkException(e, NumberFormatException.class,
					       "Out of range: " +
					       strings[idx]);
			}
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#compare(byte, byte)}.
	 */
	public void testCompareByte()
	{
		for (int i = 0; i <= 0xff; i++) {
			for (int j = 0; j <= 0xff; j++) {
				byte bi = (byte)i;
				byte bj = (byte)j;

				int result = UnsignedInteger.compare(bi, bj);
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
	 * Test case for {@link UnsignedInteger#compare(short, short)}.
	 */
	public void testCompareShort()
	{
		Random rand = new Random();

		for (int loop = 0; loop < 10000; loop++) {
			int i = rand.nextInt() & 0xffff;
			int j = rand.nextInt() & 0xffff;

			short si = (short)i;
			short sj = (short)j;
			int result = UnsignedInteger.compare(si, sj);
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
	 * Test case for {@link UnsignedInteger#compare(int, int)}.
	 */
	public void testCompareInt()
	{
		Random rand = new Random();

		for (int loop = 0; loop < 10000; loop++) {
			long i = rand.nextLong() & 0xffffffffL;
			long j = rand.nextLong() & 0xffffffffL;

			int ii = (int)i;
			int ij = (int)j;
			int result = UnsignedInteger.compare(ii, ij);
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
	 * Test case for {@link UnsignedInteger#compare(long, long)}.
	 */
	public void testCompareLong()
	{
		// Compare with zero.
		assertEquals(0, UnsignedInteger.compare(0L, 0L));

		long[] values = {
			1, 2, 3, 4, 5, 6, 7,
			0x80000000L, 0x800000001L,
			-1, -2, -3, -4, -5, -6, -7,
			Long.MIN_VALUE + 3,
			Long.MIN_VALUE + 2,
			Long.MIN_VALUE + 1,
			Long.MIN_VALUE,
			Long.MAX_VALUE - 3,
			Long.MAX_VALUE - 2,
			Long.MAX_VALUE - 1,
			Long.MAX_VALUE,
		};
		for (int idx = 0; idx < values.length; idx++) {
			int result = UnsignedInteger.compare(0, values[idx]);
			assertTrue(result < 0);

			result = UnsignedInteger.compare(values[idx], 0);
			assertTrue(result > 0);
		}

		// Compare typical unsigned long values.
		long[][] pairs = {
			// [0] = lesser, [1] = greater
			new long[]{1L, 2L},
			new long[]{0x7fffffffL, 0x80000000L},
			new long[]{0x7fffffffL, 0x7fffffffffffffffL},
			new long[]{0x7fffffffL, 0x8000000000000000L},
			new long[]{0x7fffffffL, 0xffffffff80000000L},
			new long[]{0x7fffffffL, 0xffffffffffffffeeL},
			new long[]{0x8000000000000000L, 0xffffffffffffffeeL},
			new long[]{0xffffffff80000000L, 0xffffffffffffffeeL},
			new long[]{0xffffffffffffffedL, 0xffffffffffffffeeL},
			new long[]{0xfffffffffffffff0L, 0xffffffffffffffffL},
			new long[]{0xfffffffffffffffeL, 0xffffffffffffffffL},
		};
		for (int idx = 0; idx < pairs.length; idx++) {
			long lesser = pairs[idx][0];
			long greater = pairs[idx][1];

			assertTrue(UnsignedInteger.compare(lesser, greater)
				   < 0);
			assertTrue(UnsignedInteger.compare(greater, lesser)
				   > 0);
		}

		// Compare random values.
		Random rand = new Random();
		for (int loop = 0; loop < 10000; loop++) {
			long i = rand.nextLong();
			long j = rand.nextLong();
			assertEquals(0, UnsignedInteger.compare(i, i));
			assertEquals(0, UnsignedInteger.compare(j, j));

			int result = UnsignedInteger.compare(i, j);

			int req = compare(i, j);
			if (req == 0) {
				assertEquals(0, result);
			}
			else if (req < 0) {
				assertTrue(result < 0);
			}
			else {
				assertTrue(result > 0);
			}
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#toString(byte)}.
	 */
	public void testToStringByte()
	{
		for (int i = 0; i <= 0xff; i++) {
			byte value = (byte)i;
			assertEquals(Integer.toString(i),
				     UnsignedInteger.toString(value));
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#toString(short)}.
	 */
	public void testToStringShort()
	{
		for (int i = 0; i <= 0xffff; i++) {
			short value = (short)i;
			assertEquals(Integer.toString(i),
				     UnsignedInteger.toString(value));
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#toString(int)}.
	 */
	public void testToStringInt()
	{
		LinkedList<Long> values = createIntTestData();

		for (Iterator<Long> it = values.iterator(); it.hasNext();) {
			Long lg = it.next();
			int  i = lg.intValue();
			assertEquals(lg.toString(),
				     UnsignedInteger.toString(i));
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#toString(long)}.
	 */
	public void testToStringLong()
	{
		String[] decimal = {
			"0", "1", "2", "3", "4", "5", "6", "7",
			"8", "9", "10", "11", "12", "13", "14", "15", "16",
			"268435455",
			"268435456",
			"268435457",
			"2147483646",
			"2147483647",
			"2147483648",
			"2147483649",
			"2147483650",
			"4294967293",
			"4294967294",
			"4294967295",
			"4294967296",
			"4294967297",
			"4294967298",
			"4294967299",
			"9223372036854775806",
			"9223372036854775807",
			"9223372036854775808",
			"9223372036854775809",
			"9223372036854775810",
			"17293822569102704639",
			"17293822569102704640",
			"17293822569102704641",
			"18446744073709551613",
			"18446744073709551614",
			"18446744073709551615",
		};
		long[] values = {
			0x0L, 0x1L, 0x2L, 0x3L, 0x4L, 0x5L, 0x6L, 0x7L,
			0x8L, 0x9L, 0xaL, 0xbL, 0xcL, 0xdL, 0xeL, 0xfL,
			0x10L,
			0xfffffffL,
			0x10000000L,
			0x10000001L,
			0x7ffffffeL,
			0x7fffffffL,
			0x80000000L,
			0x80000001L,
			0x80000002L,
			0xfffffffdL,
			0xfffffffeL,
			0xffffffffL,
			0x100000000L,
			0x100000001L,
			0x100000002L,
			0x100000003L,
			0x7ffffffffffffffeL,
			0x7fffffffffffffffL,
			-9223372036854775808L,		// 0x8000000000000000
			-9223372036854775807L,		// 0x8000000000000001
			-9223372036854775806L,		// 0x8000000000000002
			-1152921504606846977L,		// 0xefffffffffffffff
			-1152921504606846976L,		// 0xf000000000000000
			-1152921504606846975L,		// 0xf000000000000001
			-3L,				// 0xfffffffffffffffd
			-2L,				// 0xfffffffffffffffe
			-1L,				// 0xffffffffffffffff
		};
		for (int idx = 0; idx < values.length; idx++) {
			long value = values[idx];
			assertEquals(decimal[idx],
				     UnsignedInteger.toString(value));
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#toOctalString(byte)}.
	 */
	public void testToOctalStringByte()
	{
		for (int i = 0; i <= 0xff; i++) {
			byte value = (byte)i;
			assertEquals(Integer.toOctalString(i),
				     UnsignedInteger.toOctalString(value));
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#toOctalString(short)}.
	 */
	public void testToOctalStringShort()
	{
		for (int i = 0; i <= 0xffff; i++) {
			short value = (short)i;
			assertEquals(Integer.toOctalString(i),
				     UnsignedInteger.toOctalString(value));
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#toOctalString(int)}.
	 */
	public void testToOctalStringInt()
	{
		LinkedList<Long> values = createIntTestData();

		for (Iterator<Long> it = values.iterator(); it.hasNext();) {
			Long lg = it.next();
			long l = lg.longValue();
			int  i = lg.intValue();
			assertEquals(Long.toOctalString(l),
				     UnsignedInteger.toOctalString(i));
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#toOctalString(long)}.
	 */
	public void testToOctalStringLong()
	{
		String[] octal = {
			"0", "1", "2", "3", "4", "5", "6", "7",
			"10", "11", "12", "13", "14", "15", "16", "17",
			"20",
			"1777777777",
			"2000000000",
			"2000000001",
			"17777777776",
			"17777777777",
			"20000000000",
			"20000000001",
			"20000000002",
			"37777777775",
			"37777777776",
			"37777777777",
			"40000000000",
			"40000000001",
			"40000000002",
			"40000000003",
			"777777777777777777776",
			"777777777777777777777",
			"1000000000000000000000",
			"1000000000000000000001",
			"1000000000000000000002",
			"1677777777777777777777",
			"1700000000000000000000",
			"1700000000000000000001",
			"1777777777777777777775",
			"1777777777777777777776",
			"1777777777777777777777",
		};
		long[] values = {
			0x0L, 0x1L, 0x2L, 0x3L, 0x4L, 0x5L, 0x6L, 0x7L,
			0x8L, 0x9L, 0xaL, 0xbL, 0xcL, 0xdL, 0xeL, 0xfL,
			0x10L,
			0xfffffffL,
			0x10000000L,
			0x10000001L,
			0x7ffffffeL,
			0x7fffffffL,
			0x80000000L,
			0x80000001L,
			0x80000002L,
			0xfffffffdL,
			0xfffffffeL,
			0xffffffffL,
			0x100000000L,
			0x100000001L,
			0x100000002L,
			0x100000003L,
			0x7ffffffffffffffeL,
			0x7fffffffffffffffL,
			-9223372036854775808L,		// 0x8000000000000000
			-9223372036854775807L,		// 0x8000000000000001
			-9223372036854775806L,		// 0x8000000000000002
			-1152921504606846977L,		// 0xefffffffffffffff
			-1152921504606846976L,		// 0xf000000000000000
			-1152921504606846975L,		// 0xf000000000000001
			-3L,				// 0xfffffffffffffffd
			-2L,				// 0xfffffffffffffffe
			-1L,				// 0xffffffffffffffff
		};
		for (int idx = 0; idx < values.length; idx++) {
			long value = values[idx];
			assertEquals(octal[idx],
				     UnsignedInteger.toOctalString(value));
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#toHexString(byte)}.
	 */
	public void testToHexStringByte()
	{
		for (int i = 0; i <= 0xff; i++) {
			byte value = (byte)i;
			assertEquals(Integer.toHexString(i),
				     UnsignedInteger.toHexString(value));
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#toHexString(short)}.
	 */
	public void testToHexStringShort()
	{
		for (int i = 0; i <= 0xffff; i++) {
			short value = (short)i;
			assertEquals(Integer.toHexString(i),
				     UnsignedInteger.toHexString(value));
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#toHexString(int)}.
	 */
	public void testToHexStringInt()
	{
		LinkedList<Long> values = createIntTestData();

		for (Iterator<Long> it = values.iterator(); it.hasNext();) {
			Long lg = it.next();
			long l = lg.longValue();
			int  i = lg.intValue();
			assertEquals(Long.toHexString(l),
				     UnsignedInteger.toHexString(i));
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#toHexString(long)}.
	 */
	public void testToHexStringLong()
	{
		String[] hexadecimal = {
			"0", "1", "2", "3", "4", "5", "6", "7",
			"8", "9", "a", "b", "c", "d", "e", "f",
			"10",
			"fffffff",
			"10000000",
			"10000001",
			"7ffffffe",
			"7fffffff",
			"80000000",
			"80000001",
			"80000002",
			"fffffffd",
			"fffffffe",
			"ffffffff",
			"100000000",
			"100000001",
			"100000002",
			"100000003",
			"7ffffffffffffffe",
			"7fffffffffffffff",
			"8000000000000000",
			"8000000000000001",
			"8000000000000002",
			"efffffffffffffff",
			"f000000000000000",
			"f000000000000001",
			"fffffffffffffffd",
			"fffffffffffffffe",
			"ffffffffffffffff",
		};
		long[] values = {
			0x0L, 0x1L, 0x2L, 0x3L, 0x4L, 0x5L, 0x6L, 0x7L,
			0x8L, 0x9L, 0xaL, 0xbL, 0xcL, 0xdL, 0xeL, 0xfL,
			0x10L,
			0xfffffffL,
			0x10000000L,
			0x10000001L,
			0x7ffffffeL,
			0x7fffffffL,
			0x80000000L,
			0x80000001L,
			0x80000002L,
			0xfffffffdL,
			0xfffffffeL,
			0xffffffffL,
			0x100000000L,
			0x100000001L,
			0x100000002L,
			0x100000003L,
			0x7ffffffffffffffeL,
			0x7fffffffffffffffL,
			-9223372036854775808L,		// 0x8000000000000000
			-9223372036854775807L,		// 0x8000000000000001
			-9223372036854775806L,		// 0x8000000000000002
			-1152921504606846977L,		// 0xefffffffffffffff
			-1152921504606846976L,		// 0xf000000000000000
			-1152921504606846975L,		// 0xf000000000000001
			-3L,				// 0xfffffffffffffffd
			-2L,				// 0xfffffffffffffffe
			-1L,				// 0xffffffffffffffff
		};
		for (int idx = 0; idx < values.length; idx++) {
			long value = values[idx];
			assertEquals(hexadecimal[idx],
				     UnsignedInteger.toHexString(value));
		}
	}

	/**
	 * Test case for {@link UnsignedInteger#divide(byte, byte)} and
	 * {@link UnsignedInteger#modulo(byte, byte)}.
	 */
	public void testDivideByte()
	{
		for (int i = 0; i <= 0xff; i++) {
			for (int j = 1; j <= 0xff; j++) {
				byte bi = (byte)i;
				byte bj = (byte)j;
				int  quot = i / j;
				int  mod = i % j;
				assertEquals((byte)quot,
					     UnsignedInteger.divide(bi, bj));
				assertEquals((byte)mod,
					     UnsignedInteger.modulo(bi, bj));
				assertEquals(bi, (byte)(quot * j + mod));
			}
		}

		// Division by zero.
		Runnable[] tests = {
			new Runnable() {
				public void run()
				{
					UnsignedInteger.
						divide((byte)1, (byte)0);
				}
			},
			new Runnable() {
				public void run()
				{
					UnsignedInteger.
						modulo((byte)1, (byte)0);
				}
			},
		};

		checkDivZero(tests);
	}

	/**
	 * Test case for {@link UnsignedInteger#divide(short, short)} and
	 * {@link UnsignedInteger#modulo(short, short)}.
	 */
	public void testDivideShort()
	{
		Random rand = new Random();

		for (int loop = 0; loop < 10000; loop++) {
			int i = rand.nextInt() & 0xffff;
			int j = rand.nextInt() & 0xffff;

			if (j == 0) {
				j = 1;
			}
			int quot = i / j;
			int mod = i % j;

			short si = (short)i;
			short sj = (short)j;
			assertEquals((short)quot,
				     UnsignedInteger.divide(si, sj));
			assertEquals((short)mod,
				     UnsignedInteger.modulo(si, sj));
			assertEquals(si, (short)(quot * j + mod));
		}

		// Division by zero.
		Runnable[] tests = {
			new Runnable() {
				public void run()
				{
					UnsignedInteger.
						divide((short)1, (short)0);
				}
			},
			new Runnable() {
				public void run()
				{
					UnsignedInteger.
						modulo((short)1, (short)0);
				}
			},
		};

		checkDivZero(tests);
	}

	/**
	 * Test case for {@link UnsignedInteger#divide(int, int)} and
	 * {@link UnsignedInteger#modulo(int, int)}.
	 */
	public void testDivideInt()
	{
		Random rand = new Random();

		for (int loop = 0; loop < 10000; loop++) {
			long i = rand.nextLong() & 0xffffffffL;
			long j = rand.nextLong() & 0xffffffffL;
			if (j == 0) {
				j = 1;
			}
			long quot = i / j;
			long mod = i % j;

			int ii = (int)i;
			int ij = (int)j;
			assertEquals((int)quot,
				     UnsignedInteger.divide(ii, ij));
			assertEquals((int)mod,
				     UnsignedInteger.modulo(ii, ij));
			assertEquals(ii, (int)(quot * j + mod));
		}

		// Division by zero.
		Runnable[] tests = {
			new Runnable() {
				public void run()
				{
					int i = 1;
					int j = 0;
					UnsignedInteger.divide(i, j);
				}
			},
			new Runnable() {
				public void run()
				{
					int i = 1;
					int j = 0;
					UnsignedInteger.modulo(i, j);
				}
			},
		};

		checkDivZero(tests);
	}

	/**
	 * Test case for {@link UnsignedInteger#divide(long, long)} and
	 * {@link UnsignedInteger#modulo(long, long)}.
	 */
	public void testDivideLong()
	{
		String[][] tuples = {
			// [0] = dividend, [1] = divisor, [2] = quotient
			new String[]{"0", "1", "0"},
			new String[]{"0x1000", "3", "0x555"},
			new String[]{"0xffffffffffffffff", "17",
				     "0xf0f0f0f0f0f0f0f"},
			new String[]{"0x1000", "0xfffffffffffffff7", "0"},
			new String[]{"0xffffffff12345678",
				     "0x8000100020003000", "1"},
			new String[]{"0xffffffff12345678", "0x12345",
				     "0xe1004ffa4b6d"},
		};

		for (int idx = 0; idx < tuples.length; idx++) {
			String[] test = tuples[idx];
			long l1 = UnsignedInteger.parseLong(test[0]);
			long l2 = UnsignedInteger.parseLong(test[1]);
			long quot = UnsignedInteger.parseLong(test[2]);

			assertEquals(quot,
				     UnsignedInteger.divide(l1, l2));
			long mod = UnsignedInteger.modulo(l1, l2);
			assertEquals(l1, quot * l2 + mod);
		}

		// Division by zero.
		Runnable[] tests = {
			new Runnable() {
				public void run()
				{
					UnsignedInteger.divide(1L, 0L);
				}
			},
			new Runnable() {
				public void run()
				{
					UnsignedInteger.modulo(1L, 0L);
				}
			},
		};

		checkDivZero(tests);
	}

	/**
	 * Basic case for {@link UnsignedInteger#parseShort(String)}.
	 *
	 * @param i	Test value.
	 */
	private void parseShortTest(int i)
	{
		// Decimal
		String str = Integer.toString(i);
		short value = UnsignedInteger.parseShort(str);
		assertEquals((short)i, value);
		value = UnsignedInteger.parseShort("+" + str);
		assertEquals((short)i, value);

		// Octal
		str = "0" + Integer.toOctalString(i);
		value = UnsignedInteger.parseShort(str);
		assertEquals((short)i, value);
		value = UnsignedInteger.parseShort("+" + str);
		assertEquals((short)i, value);

		// Hexadecimal
		String hex = Integer.toHexString(i);
		str = "0x" + hex;
		value = UnsignedInteger.parseShort(str);
		assertEquals((short)i, value);
		value = UnsignedInteger.parseShort("+" + str);
		assertEquals((short)i, value);

		str = str.toUpperCase();
		value = UnsignedInteger.parseShort(str);
		assertEquals((short)i, value);
		value = UnsignedInteger.parseShort("+" + str);
		assertEquals((short)i, value);

		str = "#" + hex;
		value = UnsignedInteger.parseShort(str);
		assertEquals((short)i, value);
		value = UnsignedInteger.parseShort("+" + str);
		assertEquals((short)i, value);
	}

	/**
	 * Ensure that division by zero throws an {@code ArithmeticException}.
	 *
	 * @param tests		An array of {@code Runnable} which contains
	 *			test codes.
	 */
	private void checkDivZero(Runnable[] tests) {
		for (int idx = 0; idx < tests.length; idx++) {
			try {
				tests[idx].run();
				needException();
			}
			catch (Exception e) {
				checkException(e, ArithmeticException.class,
					       null);
			}
		}
	}

	/**
	 * Create test data for unsigned {@code int} test.
	 *
	 * @return	A {@code LinkedList} object which contains
	 *		{@code Long} objects.
	 */
	private LinkedList<Long> createIntTestData()
	{
		LinkedList<Long> values = new LinkedList<Long>();
		values.add(new Long(0));
		values.add(new Long(1));
		values.add(new Long(2));
		values.add(new Long(3));
		values.add(new Long(4));
		values.add(new Long(5));
		values.add(new Long(0x1000L));
		values.add(new Long(0x1001L));
		values.add(new Long(0x1002L));
		values.add(new Long(0x1003L));
		values.add(new Long(0x7ffffffeL));
		values.add(new Long(0x7fffffffL));
		values.add(new Long(0x80000000L));
		values.add(new Long(0x80000001L));
		values.add(new Long(0x80000002L));
		values.add(new Long(0xfffffffdL));
		values.add(new Long(0xfffffffeL));
		values.add(new Long(0xffffffffL));

		Random rand = new Random();
		for (int loop = 0; loop < 10000; loop++) {
			int i = rand.nextInt();
			values.add(new Long(UnsignedInteger.longValue(i)));
		}

		return values;
	}

	/**
	 * Compare the specified two {@code long} values.
	 * They are compared as unsigned integer.
	 *
	 * @param l1	A value to be compared.
	 * @param l2	A value to be compared.
	 * @return	A negative integer, zero, or a positive integer is
	 *		returned if {@code l1} is less than, equal to,
	 *		or greater than {@code l2} respectively.
	 */
	private static native int compare(long l1, long l2);
}
