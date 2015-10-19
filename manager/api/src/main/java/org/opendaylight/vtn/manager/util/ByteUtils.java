/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import static org.opendaylight.vtn.manager.util.NumberUtils.MASK_BYTE;

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
     * A private class to convert bit stream into the specified byte array.
     */
    private static final class BitStream {
        /**
         * The byte array to store result.
         */
        private final byte[]  result;

        /**
         * The current byte.
         */
        private byte  current;

        /**
         * The byte offset to store the next byte.
         */
        private int  resultOff;

        /**
         * The bit offset to store the next bits into the current byte.
         */
        private int  bitOff;

        /**
         * Construct a new instance that sets LSB aligned bits into the
         * specified byte array.
         *
         * @param array  A byte array to store result.
         * @param nbits  The total number of bits to be added.
         */
        private BitStream(byte[] array, int nbits) {
            result = array;
            int mod = nbits % Byte.SIZE;
            bitOff = (mod == 0) ? 0 : Byte.SIZE - mod;
        }

        /**
         * Construct a new instance that sets bits into the specified byte
         * array with specifying the start position.
         *
         * @param array  A byte array to store result.
         * @param start  Starting byte offset in the given array.
         * @param boff   Starting bit offset in the first byte of the specified
         *               byte array.
         */
        private BitStream(byte[] array, int start, int boff) {
            result = array;
            bitOff = boff;
            resultOff = start;
            if (boff != 0) {
                byte b = result[start];
                current = (byte)(b & (MASK_BYTE << (Byte.SIZE - boff)));
            }
        }

        /**
         * Add all the bits in the given byte to the bit stream.
         *
         * @param value  A byte value.
         */
        private void add(byte value) {
            byte b = (byte)((value & MASK_BYTE) >>> bitOff);
            current |= b;
            result[resultOff++] = current;
            current = (byte)(value << (Byte.SIZE - bitOff));
        }

        /**
         * Add LSB aligned bits in the given byte to the bit stream.
         *
         * @param value  A byte value.
         * @param start  The start bit offset.
         * @param nbits  The number of bits to be added.
         */
        private void add(byte value, int start, int nbits) {
            byte v = (byte)(value << start);
            byte b = (byte)((v & MASK_BYTE) >>> bitOff);
            int off = bitOff + nbits;
            if (off < Byte.SIZE) {
                int nshift = Byte.SIZE - off;
                b = (byte)(b & (MASK_BYTE << nshift));
                current |= b;
                bitOff = off;
            } else {
                current |= b;
                result[resultOff++] = current;
                bitOff = off - Byte.SIZE;
                if (bitOff > 0) {
                    int nright = Byte.SIZE - nbits;
                    int nleft = Byte.SIZE - bitOff;
                    current = (byte)(((v & MASK_BYTE) >>> nright) << nleft);
                } else {
                    current = 0;
                }
            }
        }

        /**
         * Write unwritten bits into the target array.
         *
         * @return  {@code true} if at least one bit is written.
         *          {@code false} if all added bits are already written.
         */
        private boolean flush() {
            boolean ret = (bitOff != 0);
            if (ret) {
                // Merge with a byte at the output byte.
                byte cur = (byte)(result[resultOff] << bitOff);
                cur = (byte)((cur & MASK_BYTE) >>> bitOff);

                int nshift = Byte.SIZE - bitOff;
                byte b = (byte)(((current & MASK_BYTE) >>> nshift) << nshift);
                result[resultOff] = (byte)(cur | b);
            }

            return ret;
        }
    }

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
                append(String.format("%02x", b & MASK_BYTE));
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
        if (octet < 0 || octet > MASK_BYTE) {
            throw new NumberFormatException("Octet out of range: " + hex);
        }

        return octet;
    }

    /**
     * Read the specified number of bits from the given byte array.
     *
     * @param data   A byte array.
     *               Specifying {@code null} results in undefined behavior.
     * @param off    The bit offset to start fetching bits.
     * @param nbits  The number of bits to be fetched.
     * @return  A byte array that contains fetched bits (LSB aligned).
     * @since  Beryllium
     */
    public static byte[] getBits(byte[] data, int off, int nbits) {
        int round = Byte.SIZE - 1;
        int nbytes = (nbits + round) / Byte.SIZE;
        byte[] result = new byte[nbytes];
        if (nbytes != 0) {
            // Byte offset to the first byte to be read.
            int start = off / Byte.SIZE;

            // Start bit offset in the first byte.
            int firstBit = off - (start * Byte.SIZE);

            // The last bit offset (exclusive).
            int lastoff = off + nbits;

            // End bit offset (exclusive) in the last byte.
            int lastBit = lastoff % Byte.SIZE;

            if (firstBit == 0 && lastBit == 0) {
                // No need to shift bits.
                System.arraycopy(data, start, result, 0, nbytes);
            } else {
                BitStream bst = new BitStream(result, nbits);

                // Add bits in the first byte.
                boolean moreBytes = ((firstBit + nbits) > Byte.SIZE);
                int nb = (moreBytes) ? Byte.SIZE - firstBit : nbits;
                bst.add(data[start], firstBit, nb);

                if (moreBytes) {
                    // Determine the number of bytes to be read from the
                    // source byte array.
                    int nread = (lastoff + round) / Byte.SIZE;
                    if (lastBit != 0) {
                        nread--;
                    }

                    // Add bits except for the last byte.
                    int index;
                    for (index = start + 1; index < nread; index++) {
                        bst.add(data[index]);
                    }

                    if (lastBit != 0) {
                        // Add bits in the last byte.
                        bst.add(data[index], 0, lastBit);
                    }
                }

                assert !bst.flush();
            }
        }

        return result;
    }

    /**
     * Copy bits in the specified input byte array into the specified output
     * array.
     *
     * @param output  A byte array to store bits.
     *                Specifying {@code null} results in undefined behavior.
     * @param input   A byte array to be copied.
     *                Bits in this array are expected to be aligned to LSB.
     *                Specifying {@code null} results in undefined behavior.
     * @param off     The bit offset of {@code output} to start inserting bits
     *                from {@code input}.
     * @param nbits   The number of bits to be copied.
     * @since  Beryllium
     */
    public static void setBits(byte[] output, byte[] input, int off,
                               int nbits) {
        int round = Byte.SIZE - 1;
        int nbytes = (nbits + round) / Byte.SIZE;
        if (nbytes != 0) {
            // Output byte offset to the first byte to store.
            int start = off / Byte.SIZE;

            // Output start bit offset in the first byte.
            int firstBit = off - (start * Byte.SIZE);

            // The number of bits (from LSB) in the first input byte to be
            // copied.
            int inputBits = nbits % Byte.SIZE;

            if (firstBit == 0 && inputBits == 0) {
                // No need to shift bits.
                System.arraycopy(input, 0, output, start, nbytes);
            } else {
                BitStream bst = new BitStream(output, start, firstBit);

                int index;
                if (inputBits == 0) {
                    index = 0;
                } else {
                    // Copy bits in the first byte.
                    bst.add(input[0], Byte.SIZE - inputBits, inputBits);
                    index = 1;
                }

                // Copy the rest of bits.
                for (; index < nbytes; index++) {
                    bst.add(input[index]);
                }

                // Flush unwritten bits.
                bst.flush();
            }
        }
    }
}
