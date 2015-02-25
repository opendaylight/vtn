/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link IpNetwork}.
 */
public class IpNetworkTest extends TestBase {
    /**
     * Test case for {@link IpNetwork#getInetAddress(byte[])} and
     * {@link IpNetwork#getInetAddress(String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetInetAddress() throws Exception {
        String[] v4addr = {
            "0.0.0.0",
            "127.0.0.1",
            "10.1.2.3",
            "255.255.255.255",
        };
        for (String addr: v4addr) {
            InetAddress expected = InetAddress.getByName(addr);
            byte[] bytes = expected.getAddress();
            InetAddress inet = IpNetwork.getInetAddress(bytes);
            assertEquals(expected, inet);
            assertTrue(inet instanceof Inet4Address);
            assertEquals(expected, IpNetwork.getInetAddress(addr));
        }

        String[] v6addr = {
            "::",
            "::1",
            "2001::123:abc",
            "1234:5678:abcd:efff:aabb:ccdd:eeff:9876",
        };
        for (String addr: v6addr) {
            InetAddress expected = InetAddress.getByName(addr);
            byte[] bytes = expected.getAddress();
            InetAddress inet = IpNetwork.getInetAddress(bytes);
            assertEquals(expected, inet);
            assertTrue(inet instanceof Inet6Address);
            assertEquals(expected, IpNetwork.getInetAddress(addr));
        }

        try {
            IpNetwork.getInetAddress((byte[])null);
            unexpected();
        } catch (IllegalArgumentException e) {
            assertEquals("Invalid raw IP address: null", e.getMessage());
        }

        try {
            IpNetwork.getInetAddress((String)null);
            unexpected();
        } catch (NullPointerException e) {
        }

        try {
            IpNetwork.getInetAddress("");
            unexpected();
        } catch (IllegalArgumentException e) {
            assertEquals("IP address cannot be empty.", e.getMessage());
        }

        for (int i = 0; i <= 100; i++) {
            if (i == 4 || i == 16) {
                continue;
            }

            byte[] addr = new byte[i];
            try {
                IpNetwork.getInetAddress(addr);
                unexpected();
            } catch (IllegalArgumentException e) {
                String msg = "Invalid raw IP address: " +
                    ByteUtils.toHexString(addr);
                assertEquals(msg, e.getMessage());
            }
        }

        String[] badAddrs = {
            "1234567890abcde",
            "1.2.3.4.5",
            "1.2.3.256",
            "ffabc:dd::e",
            "abc::ddee::123",
            "bad address",
        };

        for (String addr: badAddrs) {
            try {
                IpNetwork.getInetAddress(addr);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid IP address: " + addr, e.getMessage());
            }
        }
    }
}
