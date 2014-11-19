/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.net.InetAddress;
import java.util.HashSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link Inet4AddressMatch}.
 */
public class Inet4AddressMatchTest extends TestBase {
    /**
     * Test for getter methods.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetter() throws Exception {
        String[] strAddrs = {
            "0.0.0.0",
            "10.45.134.209",
            "192.168.39.17",
            "203.201.183.231",
            "255.255.255.255",
        };

        short[] badSuffixes = {
            Short.MIN_VALUE, -3, -2, -1, 0,
            32, 33, 50, 100, Short.MAX_VALUE,
        };

        for (String strAddr: strAddrs) {
            InetAddress iaddr = InetAddress.getByName(strAddr);
            int addr = MiscUtils.toInteger(iaddr);
            Inet4AddressMatch am = new Inet4AddressMatch(iaddr, null);
            assertEquals(addr, am.getAddress());
            assertEquals(iaddr, am.getInetAddress());
            assertEquals(-1, am.getMask());
            assertEquals(null, am.getCidrSuffix());

            for (short suff = 1; suff < 32; suff++) {
                int mask = -1 << (32 - suff);
                int raddr = addr & mask;
                InetAddress riaddr = MiscUtils.toInetAddress(raddr);
                Short s = Short.valueOf(suff);
                am = new Inet4AddressMatch(iaddr, s);
                assertEquals(raddr, am.getAddress());
                assertEquals(riaddr, am.getInetAddress());
                assertEquals(mask, am.getMask());
                assertEquals(s, am.getCidrSuffix());
            }

            // Specifying invalid CIDR suffix.
            for (short suff: badSuffixes) {
                Short s = Short.valueOf(suff);
                try {
                    new Inet4AddressMatch(iaddr, s);
                    unexpected();
                } catch (VTNException e) {
                    Status st = e.getStatus();
                    assertEquals(StatusCode.BADREQUEST, st.getCode());
                    assertEquals("Invalid CIDR suffix: " + suff,
                                 st.getDescription());
                }
            }
        }
    }

    /**
     * Test case for {@link Inet4AddressMatch#equals(Object)} and
     * {@link Inet4AddressMatch#hashCode()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Object> set = new HashSet<Object>();
        String[] strAddrs = {
            "0.0.0.0",
            "10.45.134.209",
            "192.168.39.17",
            "203.201.183.231",
            "255.255.255.255",
        };

        HashSet<Long> alreadySet = new HashSet<Long>();
        int count = 0;
        for (String strAddr: strAddrs) {
            InetAddress iaddr = InetAddress.getByName(strAddr);
            int addr = MiscUtils.toInteger(iaddr);
            long cookie = (long)addr << Integer.SIZE;
            if (alreadySet.add(cookie)) {
                Inet4AddressMatch am1 = new Inet4AddressMatch(iaddr, null);
                Inet4AddressMatch am2 = new Inet4AddressMatch(copy(iaddr),
                                                              null);
                testEquals(set, am1, am2);
                count++;
            }

            for (short suff = 1; suff < 32; suff++) {
                int mask = -1 << (32 - suff);
                int raddr = addr & mask;
                cookie = ((long)raddr << Integer.SIZE) | (long)suff;
                if (alreadySet.add(cookie)) {
                    Inet4AddressMatch am1 =
                        new Inet4AddressMatch(iaddr, new Short(suff));
                    Inet4AddressMatch am2 =
                        new Inet4AddressMatch(copy(iaddr), new Short(suff));
                    testEquals(set, am1, am2);
                    count++;
                }
            }
        }

        assertEquals(count, set.size());
    }

    /**
     * Test case for {@link Inet4AddressMatch#toString()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testToString() throws Exception {
        String[] strAddrs = {
            "0.0.0.0",
            "10.45.134.209",
            "192.168.39.17",
            "203.201.183.231",
            "255.255.255.255",
        };

        short[] badSuffixes = {
            Short.MIN_VALUE, -3, -2, -1, 0,
            32, 33, 50, 100, Short.MAX_VALUE,
        };

        for (String strAddr: strAddrs) {
            InetAddress iaddr = InetAddress.getByName(strAddr);
            int addr = MiscUtils.toInteger(iaddr);
            Inet4AddressMatch am = new Inet4AddressMatch(iaddr, null);
            assertEquals(strAddr, am.toString());

            for (short suff = 1; suff < 32; suff++) {
                int mask = -1 << (32 - suff);
                int raddr = addr & mask;
                InetAddress riaddr = MiscUtils.toInetAddress(raddr);
                Short s = Short.valueOf(suff);
                am = new Inet4AddressMatch(iaddr, s);
                assertEquals(riaddr.getHostAddress() + "/" + s, am.toString());
            }
        }
    }

    /**
     * Ensure that {@link Inet4AddressMatch} is serializable.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSerialize() throws Exception {
        String[] strAddrs = {
            "0.0.0.0",
            "10.45.134.209",
            "192.168.39.17",
            "203.201.183.231",
            "255.255.255.255",
        };

        short[] badSuffixes = {
            Short.MIN_VALUE, -3, -2, -1, 0,
            32, 33, 50, 100, Short.MAX_VALUE,
        };

        for (String strAddr: strAddrs) {
            InetAddress iaddr = InetAddress.getByName(strAddr);
            int addr = MiscUtils.toInteger(iaddr);
            Inet4AddressMatch am = new Inet4AddressMatch(iaddr, null);
            serializeTest(am);

            for (short suff = 1; suff < 32; suff++) {
                int mask = -1 << (32 - suff);
                int raddr = addr & mask;
                InetAddress riaddr = MiscUtils.toInetAddress(raddr);
                Short s = Short.valueOf(suff);
                am = new Inet4AddressMatch(iaddr, s);
                serializeTest(am);
            }
        }
    }

    /**
     * Test case for {@link Inet4AddressMatch#match(int)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testMatch() throws Exception {
        String[] strAddrs = {
            "0.0.0.0",
            "10.45.134.209",
            "192.168.39.17",
            "203.201.183.231",
            "255.255.255.255",
        };

        for (String strAddr: strAddrs) {
            InetAddress iaddr = InetAddress.getByName(strAddr);
            int addr = MiscUtils.toInteger(iaddr);
            Inet4AddressMatch am = new Inet4AddressMatch(iaddr, null);
            assertEquals(true, am.match(addr));
            for (int nbits = 1; nbits <= 32; nbits++) {
                int mask = -1 << (32 - nbits);
                int[] addrs = {
                    0,
                    addr & mask,
                    addr ^ mask,
                    addr | mask,
                    addr | (~mask),
                    -1,
                };
                for (int a: addrs) {
                    assertEquals(a == addr, am.match(a));
                }
            }

            for (short suff = 1; suff < 32; suff++) {
                am = new Inet4AddressMatch(iaddr, Short.valueOf(suff));
                int mask = -1 << (32 - suff);
                int raddr = addr & mask;
                int[] addrs = {
                    0,
                    addr & mask,
                    addr ^ mask,
                    addr | mask,
                    addr | (~mask),
                    -1,
                };
                for (int a: addrs) {
                    assertEquals((a & mask) == raddr, am.match(a));
                }
            }
        }
    }
}
