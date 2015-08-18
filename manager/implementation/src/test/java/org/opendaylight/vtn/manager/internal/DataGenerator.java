/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.util.ArrayList;

/**
 * Collection of test data.
 */
public final class DataGenerator {
    /**
     * An array of MAC addresses for test.
     */
    private static final byte[][] MAC_ADDRESSES = {
        new byte[]{
            (byte)0x14, (byte)0x64, (byte)0xa2,
            (byte)0x00, (byte)0xcb, (byte)0x6a,
        },
        new byte[]{
            (byte)0x9a, (byte)0xa1, (byte)0x45,
            (byte)0x37, (byte)0x1f, (byte)0x5e,
        },
        new byte[]{
            (byte)0x30, (byte)0x3b, (byte)0xf4,
            (byte)0x02, (byte)0x8d, (byte)0x30,
        },
        new byte[]{
            (byte)0xbc, (byte)0xe6, (byte)0xc5,
            (byte)0x29, (byte)0xb4, (byte)0xc4,
        },
        new byte[]{
            (byte)0xc8, (byte)0x5d, (byte)0xab,
            (byte)0xe8, (byte)0xf2, (byte)0xd4,
        },
        new byte[]{
            (byte)0xe4, (byte)0x8f, (byte)0x77,
            (byte)0xc6, (byte)0x84, (byte)0x05,
        },
        new byte[]{
            (byte)0x91, (byte)0x23, (byte)0x21,
            (byte)0xf7, (byte)0xdd, (byte)0xaf,
        },
        new byte[]{
            (byte)0x9c, (byte)0xab, (byte)0x65,
            (byte)0xe4, (byte)0x10, (byte)0x05,
        },
    };

    /**
     * An array of IP addresses for test.
     */
    private static final InetAddress[] INET_ADDRESSES;

    /**
     * An array of TCP/UDP port numbers for test.
     */
    private static final int[]  PORT_NUMBERS = {
        1, 3, 22, 123, 294, 512, 1045, 3041, 12835, 18346, 23841, 29347,
        31293, 32767, 33124, 41982, 51293, 59812, 60212, 63321, 65534, 65535,
    };

    /**
     * Initialize static fields.
     */
    static {
        ArrayList<InetAddress> addrs = new ArrayList<InetAddress>();
        try {
            addrs.add(InetAddress.getByName("10.1.2.3"));
            addrs.add(InetAddress.getByName("192.168.35.129"));
            addrs.add(InetAddress.getByName("172.31.159.254"));
            addrs.add(InetAddress.getByName("203.183.34.99"));
            addrs.add(InetAddress.getByName("10.125.187.31"));
            addrs.add(InetAddress.getByName("59.38.222.45"));
            addrs.add(InetAddress.getByName("98.31.76.245"));
            addrs.add(InetAddress.getByName("245.31.195.74"));
            INET_ADDRESSES = addrs.toArray(new InetAddress[addrs.size()]);
        } catch (Exception e) {
            throw new IllegalStateException(
                "Failed to initialize IP address", e);
        }
    }

    /**
     * Cursor that points MAC address in {@link #MAC_ADDRESSES}.
     */
    private int  macIndex;

    /**
     * Cursor that points IP address in {@link #INET_ADDRESSES}.
     */
    private int  ipIndex;

    /**
     * Cursor that points port number in {@link #PORT_NUMBERS}.
     */
    private int  portIndex;

    /**
     * Return MAC address for test.
     *
     * @return  A byte array which represents a MAC address.
     */
    public byte[] getMacAddress() {
        byte[] ret = MAC_ADDRESSES[macIndex].clone();
        macIndex++;
        if (macIndex >= MAC_ADDRESSES.length) {
            macIndex = 0;
        }

        return ret;
    }

    /**
     * Return IPv4 address for test.
     *
     * @return  An {@link InetAddress} instance.
     */
    public InetAddress getInetAddress() {
        InetAddress addr = INET_ADDRESSES[ipIndex];
        ipIndex++;
        if (ipIndex >= INET_ADDRESSES.length) {
            ipIndex = 0;
        }

        return addr;
    }

    /**
     * Return TCP/UDP port number for test.
     *
     * @return  A port number.
     */
    public int getPort() {
        int port = PORT_NUMBERS[portIndex];
        portIndex++;
        if (portIndex >= PORT_NUMBERS.length) {
            portIndex = 0;
        }

        return port;
    }

    /**
     * Return ICMP type/code value for test.
     *
     * @return  A short value for ICMP type/code test.
     */
    public Short getIcmpValue() {
        return Short.valueOf((short)(getPort() & 0xff));
    }
}
