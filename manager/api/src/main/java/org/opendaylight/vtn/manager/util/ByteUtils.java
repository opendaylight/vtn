/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

/**
 * {@code ByteUtils} class is a collection of utility class fields and methods
 * for byte array handling.
 *
 * @since  Lithium
 */
public final class ByteUtils {
    /**
     * A character that separates octets in a hex string.
     */
    public static final char  HEX_SEPARATOR_CHAR = ':';

    /**
     * Separator for octets in a hex string.
     */
    public static final String  HEX_SEPARATOR =
        String.valueOf(HEX_SEPARATOR_CHAR);

    /**
     * Radix to be used in intepreting a hex string.
     */
    public static final int  HEX_RADIX = 16;

    /**
     * Private constructor that protects this class from instantiating.
     */
    private ByteUtils() {}

    /**
     * Convert the given byte array into hex string with ":" inserted.
     *
     * <p>
     *   Note that this method returns {@code null} if {@code null} is passed.
     * </p>
     *
     * @param bytes  A byte array.
     * @return  A hex string.
     */
    public static String toHexString(byte[] bytes) {
        if (bytes == null) {
            return null;
        }

        String sep = "";
        StringBuilder builder = new StringBuilder();
        for (byte b: bytes) {
            builder.append(sep).
                append(String.format("%02x", b & NumberUtils.MASK_BYTE));
            sep = HEX_SEPARATOR;
        }
        return builder.toString();
    }

    /**
     * Convert the given hex string with ":" inserted into a byte array.
     *
     * <ul>
     *   <li>{@code null} is returned if {@code null} is passed.</li>
     *   <li>An empty byte array is returned if an empty string is passed.</li>
     * </ul>
     *
     * @param hex  A hex string with ":" inserted.
     * @return  A byte array.
     * @throws NumberFormatException
     *    The given string is not a hex string.
     */
    public static byte[] toBytes(String hex) {
        if (hex == null) {
            return null;
        }
        if (hex.isEmpty()) {
            return new byte[0];
        }

        String[] octets = hex.split(HEX_SEPARATOR);
        byte[] bytes = new byte[octets.length];
        for (int i = 0; i < octets.length; i++) {
            bytes[i] = (byte)parseHexOctet(octets[i]);
        }
        return bytes;
    }

    /**
     * Convert the given hex string as unsigned byte.
     *
     * @param hex  A hex string.
     * @return  An integer value.
     * @throws NumberFormatException
     *    The given string could not be converted into an unsigned byte.
     */
    public static int parseHexOctet(String hex) {
        int octet = Integer.valueOf(hex, HEX_RADIX).intValue();
        if (octet < 0 || octet > NumberUtils.MASK_BYTE) {
            throw new NumberFormatException("Octet out of range: " + hex);
        }

        return octet;
    }
}
