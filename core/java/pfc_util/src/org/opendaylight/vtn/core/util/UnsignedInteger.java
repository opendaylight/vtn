/*
 * Copyright (c) 2012-2013 NEC Corporation
 * All rights reserved.
 * 
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.core.util;

/**
 * <p>
 *   The {@code UnsignedInteger} class is a collection of static methods
 *   to handle unsigned integer in Java program.
 *   It can not be instantiated.
 * </p>
 *
 * @since	C10
 */
public final class UnsignedInteger
{
	/**
	 * The maximum value of unsigned 8-bit integer.
	 */
	public final static long  MAX_UINT8 = 0xffL;

	/**
	 * The maximum value of unsigned 16-bit integer.
	 */
	public final static long  MAX_UINT16 = 0xffffL;

	/**
	 * The maximum value of unsigned 32-bit integer.
	 */
	public final static long  MAX_UINT32 = 0xffffffffL;

	/**
	 * Load native library.
	 */
	static {
		PfcUtil.load();
	}

	/**
	 * Private constructor which will never be called.
	 */
	private UnsignedInteger()
	{
	}

	/**
	 * Convert the specified {@code byte} value to {@code short}.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value to be converted.
	 * @return		An unsigned value.
	 */
	public static short shortValue(byte value)
	{
		return (short)((int)value & 0xff);
	}

	/**
	 * Convert the specified {@code byte} value to {@code int}.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value to be converted.
	 * @return		An unsigned value.
	 */
	public static int intValue(byte value)
	{
		return (int)value & 0xff;
	}

	/**
	 * Convert the specified {@code short} value to {@code int}.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value to be converted.
	 * @return		An unsigned value.
	 */
	public static int intValue(short value)
	{
		return (int)value & 0xffff;
	}

	/**
	 * Convert the specified {@code byte} value to {@code long}.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value to be converted.
	 * @return		An unsigned value.
	 */
	public static long longValue(byte value)
	{
		return (long)value & 0xffL;
	}

	/**
	 * Convert the specified {@code short} value to {@code long}.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value to be converted.
	 * @return		An unsigned value.
	 */
	public static long longValue(short value)
	{
		return (long)value & 0xffffL;
	}

	/**
	 * Convert the specified {@code int} value to {@code long}.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value to be converted.
	 * @return		An unsigned value.
	 */
	public static long longValue(int value)
	{
		return (long)value & 0xffffffffL;
	}

	/**
	 * Convert the specified {@code byte} value to {@code float}.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value to be converted.
	 * @return		A converted value.
	 */
	public static float floatValue(byte value)
	{
		int i = (int)value & 0xff;

		return (float)i;
	}

	/**
	 * Convert the specified {@code short} value to {@code float}.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value to be converted.
	 * @return		A converted value.
	 */
	public static float floatValue(short value)
	{
		int i = (int)value & 0xffff;

		return (float)i;
	}

	/**
	 * Convert the specified {@code int} value to {@code float}.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value to be converted.
	 * @return		A converted value.
	 */
	public static float floatValue(int value)
	{
		long l = (long)value & 0xffffffffL;

		return (float)l;
	}

	/**
	 * Convert the specified {@code long} value to {@code float}.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value to be converted.
	 * @return		A converted value.
	 */
	public static native float floatValue(long value);

	/**
	 * Convert the specified {@code byte} value to {@code double}.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value to be converted.
	 * @return		A converted value.
	 */
	public static double doubleValue(byte value)
	{
		int i = (int)value & 0xff;

		return (double)i;
	}

	/**
	 * Convert the specified {@code short} value to {@code double}.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value to be converted.
	 * @return		A converted value.
	 */
	public static double doubleValue(short value)
	{
		int i = (int)value & 0xffff;

		return (double)i;
	}

	/**
	 * Convert the specified {@code int} value to {@code double}.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value to be converted.
	 * @return		A converted value.
	 */
	public static double doubleValue(int value)
	{
		long l = (long)value & 0xffffffffL;

		return (double)l;
	}

	/**
	 * Convert the specified {@code long} value to {@code double}.
	 * The specified value is treated as unsigned integer.
	 *
	 * @param value		A value to be converted.
	 * @return		A converted value.
	 */
	public static native double doubleValue(long value);

	/**
	 * <p>
	 *   Parse the specified string as unsigned 8-bit integer.
	 * </p>
	 * <p>
	 *   The radix of the value is determined by prefix of the string.
	 * </p>
	 * <ol>
	 *   <li>
	 *     If the string starts with {@code "0x"}, {@code "0X"} or
	 *     {@code "#"}, the string is parsed in base 16.
	 *   </li>
	 *   <li>
	 *     If the string starts with {@code "0"}, the string is parsed
	 *     in base 8.
	 *   </li>
	 *   <li>
	 *     Otherwise the string is parsed in base 10.
	 *   </li>
	 * </ol>
	 *
	 * @param str	A string to be parsed.
	 * @return	A parsed unsigned 8-bit integer.
	 * @throws NullPointerException
	 *	{@code str} is null.
	 * @throws NumberFormatException
	 *	{@code str} does not contain parsable value.
	 */
	public static byte parseByte(String str)
	{
		int i = UnsignedInteger.parseInt(str);

		if (((long)i & 0xffffffffL) > MAX_UINT8) {
			throw new NumberFormatException
				("Out of range: " + str);
		}

		return (byte)i;
	}

	/**
	 * <p>
	 *   Parse the specified string as unsigned 16-bit integer.
	 * </p>
	 * <p>
	 *   The radix of the value is determined by prefix of the string.
	 * </p>
	 * <ol>
	 *   <li>
	 *     If the string starts with {@code "0x"}, {@code "0X"} or
	 *     {@code "#"}, the string is parsed in base 16.
	 *   </li>
	 *   <li>
	 *     If the string starts with {@code "0"}, the string is parsed
	 *     in base 8.
	 *   </li>
	 *   <li>
	 *     Otherwise the string is parsed in base 10.
	 *   </li>
	 * </ol>
	 *
	 * @param str	A string to be parsed.
	 * @return	A parsed unsigned 16-bit integer.
	 * @throws NullPointerException
	 *	{@code str} is null.
	 * @throws NumberFormatException
	 *	{@code str} does not contain parsable value.
	 */
	public static short parseShort(String str)
	{
		int i = UnsignedInteger.parseInt(str);

		if (((long)i & 0xffffffffL) > MAX_UINT16) {
			throw new NumberFormatException
				("Out of range: " + str);
		}

		return (short)i;
	}

	/**
	 * <p>
	 *   Parse the specified string as unsigned 32-bit integer.
	 * </p>
	 * <p>
	 *   The radix of the value is determined by prefix of the string.
	 * </p>
	 * <ol>
	 *   <li>
	 *     If the string starts with {@code "0x"}, {@code "0X"} or
	 *     {@code "#"}, the string is parsed in base 16.
	 *   </li>
	 *   <li>
	 *     If the string starts with {@code "0"}, the string is parsed
	 *     in base 8.
	 *   </li>
	 *   <li>
	 *     Otherwise the string is parsed in base 10.
	 *   </li>
	 * </ol>
	 *
	 * @param str	A string to be parsed.
	 * @return	A parsed unsigned 32-bit integer.
	 * @throws NullPointerException
	 *	{@code str} is null.
	 * @throws NumberFormatException
	 *	{@code str} does not contain parsable value.
	 */
	public static native int parseInt(String str);

	/**
	 * <p>
	 *   Parse the specified string as unsigned 64-bit integer.
	 * </p>
	 * <p>
	 *   The radix of the value is determined by prefix of the string.
	 * </p>
	 * <ol>
	 *   <li>
	 *     If the string starts with {@code "0x"}, {@code "0X"} or
	 *     {@code "#"}, the string is parsed in base 16.
	 *   </li>
	 *   <li>
	 *     If the string starts with {@code "0"}, the string is parsed
	 *     in base 8.
	 *   </li>
	 *   <li>
	 *     Otherwise the string is parsed in base 10.
	 *   </li>
	 * </ol>
	 *
	 * @param str	A string to be parsed.
	 * @return	A parsed unsigned 64-bit integer.
	 * @throws NullPointerException
	 *	{@code str} is null.
	 * @throws NumberFormatException
	 *	{@code str} does not contain parsable value.
	 */
	public static native long parseLong(String str);

	/**
	 * Compare the given two unsigned 8-bit integer values.
	 *
	 * @param b1	A value to be compared.
	 * @param b2	A value to be compared.
	 * @return	A negative integer, zero, or a positive integer is
	 *		returned if {@code b1} is less than, equal to,
	 *		or greater than {@code b2} respectively.
	 */
	public static int compare(byte b1, byte b2)
	{
		int  i1 = (int)b1 & 0xff;
		int  i2 = (int)b2 & 0xff;

		return i1 - i2;
	}

	/**
	 * Compare the given two unsigned 16-bit integer values.
	 *
	 * @param s1	A value to be compared.
	 * @param s2	A value to be compared.
	 * @return	A negative integer, zero, or a positive integer is
	 *		returned if {@code s1} is less than, equal to,
	 *		or greater than {@code s2} respectively.
	 */
	public static int compare(short s1, short s2)
	{
		int  i1 = (int)s1 & 0xffff;
		int  i2 = (int)s2 & 0xffff;

		return i1 - i2;
	}

	/**
	 * Compare the given two unsigned 32-bit integer values.
	 *
	 * @param i1	A value to be compared.
	 * @param i2	A value to be compared.
	 * @return	A negative integer, zero, or a positive integer is
	 *		returned if {@code i1} is less than, equal to,
	 *		or greater than {@code i2} respectively.
	 */
	public static int compare(int i1, int i2)
	{
		if (i1 == i2) {
			return 0;
		}

		long l1 = (long)i1 & 0xffffffffL;
		long l2 = (long)i2 & 0xffffffffL;

		return (l1 < l2) ? -1 : 1;
	}

	/**
	 * Compare the given two unsigned 64-bit integer values.
	 *
	 * @param l1	A value to be compared.
	 * @param l2	A value to be compared.
	 * @return	A negative integer, zero, or a positive integer is
	 *		returned if {@code l1} is less than, equal to,
	 *		or greater than {@code l2} respectively.
	 */
	public static int compare(long l1, long l2)
	{
		if (l1 == l2) {
			return 0;
		}

		long msb1 = l1 >>> 63;
		long msb2 = l2 >>> 63;
		if (msb1 != msb2) {
			return (msb1 == 0) ? -1 : 1;
		}

		long body1 = l1 & 0x7fffffffffffffffL;
		long body2 = l2 & 0x7fffffffffffffffL;

		return (body1 < body2) ? -1 : 1;
	}

	/**
	 * <p>
	 *   Return a new string which represents the specified {@code byte}
	 *   value.
	 * </p>
	 * <p>
	 *   The specified value is treated as unsigned integer, and the radix
	 *   is always 10.
	 * </p>
	 *
	 * @param value		A value to be converted.
	 * @return		The string representation of the specified
	 *			value.
	 */
	public static String toString(byte value)
	{
		return Short.toString(UnsignedInteger.shortValue(value));
	}

	/**
	 * <p>
	 *   Return a new string which represents the specified {@code short}
	 *   value.
	 * </p>
	 * <p>
	 *   The specified value is treated as unsigned integer, and the radix
	 *   is always 10.
	 * </p>
	 *
	 * @param value		A value to be converted.
	 * @return		The string representation of the specified
	 *			value.
	 */
	public static String toString(short value)
	{
		return Integer.toString(UnsignedInteger.intValue(value));
	}

	/**
	 * <p>
	 *   Return a new string which represents the specified {@code int}
	 *   value.
	 * </p>
	 * </p>
	 *   The specified value is treated as unsigned integer, and the radix
	 *   is always 10.
	 * </p>
	 *
	 * @param value		A value to be converted.
	 * @return		The string representation of the specified
	 *			value.
	 */
	public static String toString(int value)
	{
		return Long.toString(UnsignedInteger.longValue(value));
	}

	/**
	 * <p>
	 *   Return a new string which represents the specified {@code long}
	 *   value.
	 * </p>
	 * <p>
	 *   The specified value is treated as unsigned integer, and the radix
	 *   is always 10.
	 * </p>
	 *
	 * @param value		A value to be converted.
	 * @return		The string representation of the specified
	 *			value.
	 */
	public static native String toString(long value);

	/**
	 * <p>
	 *   Return a new octal string which represents the specified
	 *   {@code byte} value.
	 * </p>
	 * <p>
	 *   The specified value is treated as unsigned integer.
	 *   Note that a returned string does not starts with {@code "0"}
	 *   unless {@code value} is zero.
	 * </p>
	 *
	 * @param value		A value to be converted.
	 * @return		The octal string representation of the
	 *			specified value.
	 */
	public static String toOctalString(byte value)
	{
		return Integer.toOctalString(UnsignedInteger.intValue(value));
	}

	/**
	 * <p>
	 *   Return a new octal string which represents the specified
	 *   {@code short} value.
	 * </p>
	 * <p>
	 *   The specified value is treated as unsigned integer.
	 *   Note that a returned string does not starts with {@code "0"}
	 *   unless {@code value} is zero.
	 * </p>
	 *
	 * @param value		A value to be converted.
	 * @return		The octal string representation of the
	 *			specified value.
	 */
	public static String toOctalString(short value)
	{
		return Integer.toOctalString(UnsignedInteger.intValue(value));
	}

	/**
	 * <p>
	 *   Return a new octal string which represents the specified
	 *   {@code int} value.
	 * </p>
	 * <p>
	 *   The specified value is treated as unsigned integer.
	 *   Note that a returned string does not starts with {@code "0"}
	 *   unless {@code value} is zero.
	 * </p>
	 *
	 * @param value		A value to be converted.
	 * @return		The octal string representation of the
	 *			specified value.
	 */
	public static String toOctalString(int value)
	{
		return Integer.toOctalString(value);
	}

	/**
	 * <p>
	 *   Return a new octal string which represents the specified
	 *   {@code long} value.
	 * </p>
	 * <p>
	 *   The specified value is treated as unsigned integer.
	 *   Note that a returned string does not starts with {@code "0"}
	 *   unless {@code value} is zero.
	 * </p>
	 *
	 * @param value		A value to be converted.
	 * @return		The octal string representation of the
	 *			specified value.
	 */
	public static String toOctalString(long value)
	{
		return Long.toOctalString(value);
	}

	/**
	 * <p>
	 *   Return a new hexadecimal string which represents the specified
	 *   {@code byte} value.
	 * </p>
	 * <p>
	 *   The specified value is treated as unsigned integer.
	 *   Note that a returned string does not starts with {@code "0x"},
	 *   and it does contains only digits and lower letters from
	 *   {@code 'a'} to {@code 'f'}.
	 * </p>
	 *
	 * @param value		A value to be converted.
	 * @return		The hexadecimal string representation of the
	 *			specified value.
	 */
	public static String toHexString(byte value)
	{
		return Integer.toHexString(UnsignedInteger.intValue(value));
	}

	/**
	 * <p>
	 *   Return a new hexadecimal string which represents the specified
	 *   {@code short} value.
	 * </p>
	 * <p>
	 *   The specified value is treated as unsigned integer.
	 *   Note that a returned string does not starts with {@code "0x"},
	 *   and it does contains only digits and lower letters from
	 *   {@code 'a'} to {@code 'f'}.
	 * </p>
	 *
	 * @param value		A value to be converted.
	 * @return		The hexadecimal string representation of the
	 *			specified value.
	 */
	public static String toHexString(short value)
	{
		return Integer.toHexString(UnsignedInteger.intValue(value));
	}

	/**
	 * <p>
	 *   Return a new hexadecimal string which represents the specified
	 *   {@code int} value.
	 * </p>
	 * <p>
	 *   The specified value is treated as unsigned integer.
	 *   Note that a returned string does not starts with {@code "0x"},
	 *   and it does contains only digits and lower letters from
	 *   {@code 'a'} to {@code 'f'}.
	 * </p>
	 *
	 * @param value		A value to be converted.
	 * @return		The hexadecimal string representation of the
	 *			specified value.
	 */
	public static String toHexString(int value)
	{
		return Integer.toHexString(value);
	}

	/**
	 * <p>
	 *   Return a new hexadecimal string which represents the specified
	 *   {@code long} value.
	 * </p>
	 * <p>
	 *   The specified value is treated as unsigned integer.
	 *   Note that a returned string does not starts with {@code "0x"},
	 *   and it does contains only digits and lower letters from
	 *   {@code 'a'} to {@code 'f'}.
	 * </p>
	 *
	 * @param value		A value to be converted.
	 * @return		The hexadecimal string representation of the
	 *			specified value.
	 */
	public static String toHexString(long value)
	{
		return Long.toHexString(value);
	}

	/**
	 * Divide the specified {@code byte} value.
	 * Both arguments are treated as unsigned integer.
	 *
	 * @param dividend	A dividend.
	 * @param divisor	A divisor.
	 * @return		The quotient of {@code dividend} divided by
	 *			{@code divisor}.
	 * @throws ArithmeticException
	 *	{@code divisor} is zero.
	 */
	public static byte divide(byte dividend, byte divisor)
	{
		int dvd = (int)dividend & 0xff;
		int dvs = (int)divisor & 0xff;

		return (byte)(dvd / dvs);
	}

	/**
	 * Divide the specified {@code short} value.
	 * Both arguments are treated as unsigned integer.
	 *
	 * @param dividend	A dividend.
	 * @param divisor	A divisor.
	 * @return		The quotient of {@code dividend} divided by
	 *			{@code divisor}.
	 * @throws ArithmeticException
	 *	{@code divisor} is zero.
	 */
	public static short divide(short dividend, short divisor)
	{
		int dvd = (int)dividend & 0xffff;
		int dvs = (int)divisor & 0xffff;

		return (short)(dvd / dvs);
	}

	/**
	 * Divide the specified {@code int} value.
	 * Both arguments are treated as unsigned integer.
	 *
	 * @param dividend	A dividend.
	 * @param divisor	A divisor.
	 * @return		The quotient of {@code dividend} divided by
	 *			{@code divisor}.
	 * @throws ArithmeticException
	 *	{@code divisor} is zero.
	 */
	public static int divide(int dividend, int divisor)
	{
		long dvd = (long)dividend & 0xffffffffL;
		long dvs = (long)divisor & 0xffffffffL;

		return (int)(dvd / dvs);
	}

	/**
	 * Divide the specified {@code long} value.
	 * Both arguments are treated as unsigned integer.
	 *
	 * @param dividend	A dividend.
	 * @param divisor	A divisor.
	 * @return		The quotient of {@code dividend} divided by
	 *			{@code divisor}.
	 * @throws ArithmeticException
	 *	{@code divisor} is zero.
	 */
	public static native long divide(long dividend, long divisor);

	/**
	 * Determine the remainder of division of {@code byte} values.
	 * Both arguments are treated as unsigned integer.
	 *
	 * @param dividend	A dividend.
	 * @param divisor	A divisor.
	 * @return		The remainder of {@code dividend} divided by
	 *			{@code divisor}.
	 * @throws ArithmeticException
	 *	{@code divisor} is zero.
	 */
	public static byte modulo(byte dividend, byte divisor)
	{
		int dvd = (int)dividend & 0xff;
		int dvs = (int)divisor & 0xff;

		return (byte)(dvd % dvs);
	}

	/**
	 * Determine the remainder of division of {@code short} values.
	 * Both arguments are treated as unsigned integer.
	 *
	 * @param dividend	A dividend.
	 * @param divisor	A divisor.
	 * @return		The remainder of {@code dividend} divided by
	 *			{@code divisor}.
	 * @throws ArithmeticException
	 *	{@code divisor} is zero.
	 */
	public static short modulo(short dividend, short divisor)
	{
		int dvd = (int)dividend & 0xffff;
		int dvs = (int)divisor & 0xffff;

		return (short)(dvd % dvs);
	}

	/**
	 * Determine the remainder of division of {@code int} values.
	 * Both arguments are treated as unsigned integer.
	 *
	 * @param dividend	A dividend.
	 * @param divisor	A divisor.
	 * @return		The remainder of {@code dividend} divided by
	 *			{@code divisor}.
	 * @throws ArithmeticException
	 *	{@code divisor} is zero.
	 */
	public static int modulo(int dividend, int divisor)
	{
		long dvd = (long)dividend & 0xffffffffL;
		long dvs = (long)divisor & 0xffffffffL;

		return (int)(dvd % dvs);
	}

	/**
	 * Determine the remainder of division of {@code long} values.
	 * Both arguments are treated as unsigned integer.
	 *
	 * @param dividend	A dividend.
	 * @param divisor	A divisor.
	 * @return		The remainder of {@code dividend} divided by
	 *			{@code divisor}.
	 * @throws ArithmeticException
	 *	{@code divisor} is zero.
	 */
	public static native long modulo(long dividend, long divisor);
}
