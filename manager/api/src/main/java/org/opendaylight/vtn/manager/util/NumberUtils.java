/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import java.math.BigInteger;

/**
 * {@code NumberUtils} class is a collection of utility class fields and
 * methods for number handling.
 *
 * @since  Lithium
 */
public final class NumberUtils {
    /**
     * A prime number used to calculate hash code.
     */
    public static final int  HASH_PRIME = 31;

    /**
     * The number of octets in an integer value.
     */
    public static final int  NUM_OCTETS_INTEGER = Integer.SIZE / Byte.SIZE;

    /**
     * A mask value which represents all bits in a byte value.
     */
    public static final int  MASK_BYTE = (1 << Byte.SIZE) - 1;

    /**
     * A mask value which represents all bits in a short value.
     */
    public static final int  MASK_SHORT = (1 << Short.SIZE) - 1;

    /**
     * A mask value which represents all bits in an integer value.
     */
    public static final long  MASK_INTEGER = (1L << Integer.SIZE) - 1L;

    /**
     * The number of bits to be shifted to get the first octet in an integer.
     */
    private static final int  INT_SHIFT_OCTET1 = 24;

    /**
     * The number of bits to be shifted to get the second octet in an integer.
     */
    private static final int  INT_SHIFT_OCTET2 = 16;

    /**
     * The number of bits to be shifted to get the third octet in an integer.
     */
    private static final int  INT_SHIFT_OCTET3 = 8;

    /**
     * Private constructor that protects this class from instantiating.
     */
    private NumberUtils() {}

    /**
     * Return a hash code of the given long integer value.
     *
     * @param value  A long integer value.
     * @return  A hash code of the given value.
     */
    public static int hashCode(long value) {
        return (int)(value ^ (value >>> Integer.SIZE));
    }

    /**
     * Return a hash code of the given double value.
     *
     * @param value  A double value.
     * @return  A hash code of the given value.
     */
    public static int hashCode(double value) {
        return hashCode(Double.doubleToLongBits(value));
    }

    /**
     * Set an integer value into the given byte array in network byte order.
     *
     * @param array  A byte array.
     * @param off    Index of {@code array} to store value.
     * @param value  An integer value.
     */
    public static void setInt(byte[] array, int off, int value) {
        int index = off;
        array[index++] = (byte)(value >>> INT_SHIFT_OCTET1);
        array[index++] = (byte)(value >>> INT_SHIFT_OCTET2);
        array[index++] = (byte)(value >>> INT_SHIFT_OCTET3);
        array[index] = (byte)value;
    }

    /**
     * Set a short integer value into the given byte array in network byte
     * order.
     *
     * @param array  A byte array.
     * @param off    Index of {@code array} to store value.
     * @param value  A short integer value.
     */
    public static void setShort(byte[] array, int off, short value) {
        array[off] = (byte)(value >>> Byte.SIZE);
        array[off + 1] = (byte)value;
    }

    /**
     * Convert a 4 bytes array into an integer number.
     *
     * @param b  A 4 bytes array.
     * @return   An integer number.
     * @throws NullPointerException
     *    {@code b} is {@code null}.
     * @throws IllegalArgumentException
     *    The length of {@code b} is not 4.
     */
    public static int toInteger(byte[] b) {
        if (b.length != NUM_OCTETS_INTEGER) {
            throw new IllegalArgumentException(
                "Invalid byte array length: " + b.length);
        }

        int index = 0;
        int value = (b[index++] & MASK_BYTE) << INT_SHIFT_OCTET1;
        value |= (b[index++] & MASK_BYTE) << INT_SHIFT_OCTET2;
        value |= (b[index++] & MASK_BYTE) << INT_SHIFT_OCTET3;
        return value | (b[index] & MASK_BYTE);
    }

    /**
     * Convert an integer value into an byte array.
     *
     * @param i  An integer value.
     * @return   A converted byte array.
     */
    public static byte[] toBytes(int i) {
        byte[] b = new byte[NUM_OCTETS_INTEGER];
        setInt(b, 0, i);
        return b;
    }

    /**
     * Return the unsigned value of the given byte value.
     *
     * @param b  A byte value.
     * @return   An integer value which represents the unsigned byte value.
     */
    public static int getUnsigned(byte b) {
        return (b & MASK_BYTE);
    }

    /**
     * Return the unsigned value of the given short value.
     *
     * @param s  A short value.
     * @return   An integer value which represents the unsigned short value.
     */
    public static int getUnsigned(short s) {
        return (s & MASK_SHORT);
    }

    /**
     * Return the unsigned value of the given integer value.
     *
     * @param i  An integer value.
     * @return   A long integer value which represents the unsigned integer
     *           value.
     */
    public static long getUnsigned(int i) {
        return ((long)i & MASK_INTEGER);
    }

    /**
     * Return the unsigned value of the given long integer value.
     *
     * @param l  A long integer value.
     * @return   A {@link BigInteger} instance which represents the unsigned
     *           long integer value.
     */
    public static BigInteger getUnsigned(long l) {
        BigInteger bi;
        if (l >= 0) {
            bi = BigInteger.valueOf(l);
        } else {
            String hex = Long.toHexString(l);
            bi = new BigInteger(hex, ByteUtils.HEX_RADIX);
        }

        return bi;
    }

    /**
     * Convert the given number into a {@link Byte} instance.
     *
     * @param num  A {@link Number} instance.
     * @return  A {@link Byte} instance equivalent to the given number.
     *          {@code null} if {@code num} is {@code null}.
     */
    public static Byte toByte(Number num) {
        return (num == null) ? null : Byte.valueOf(num.byteValue());
    }

    /**
     * Convert the given number into a {@link Short} instance.
     *
     * @param num  A {@link Number} instance.
     * @return  A {@link Short} instance equivalent to the given number.
     *          {@code null} if {@code num} is {@code null}.
     */
    public static Short toShort(Number num) {
        return (num == null) ? null : Short.valueOf(num.shortValue());
    }

    /**
     * Convert the given number into a {@link Integer} instance.
     *
     * @param num  A {@link Number} instance.
     * @return  An {@link Integer} instance equivalent to the given number.
     *          {@code null} if {@code num} is {@code null}.
     */
    public static Integer toInteger(Number num) {
        return (num == null) ? null : Integer.valueOf(num.intValue());
    }

    /**
     * Convert the given number into a {@link Long} instance.
     *
     * @param num  A {@link Number} instance.
     * @return  A {@link Long} instance equivalent to the given number.
     *          {@code null} if {@code num} is {@code null}.
     */
    public static Long toLong(Number num) {
        return (num == null) ? null : Long.valueOf(num.longValue());
    }

    /**
     * Determine whether the given double numbers are identical.
     *
     * @param d1  The first double number to be compared.
     * @param d2  The second double number to be compared.
     * @return  {@code true} only if the given two double numbers are
     *          identical.
     * @see Double#equals(Object)
     */
    public static boolean equalsDouble(double d1, double d2) {
        return (Double.doubleToLongBits(d1) == Double.doubleToLongBits(d2));
    }
}
