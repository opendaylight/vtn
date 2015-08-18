/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.util.Random;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.Address;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv4;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv4Builder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv6;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv6Builder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpPrefix;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Prefix;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv6Prefix;

/**
 * JUnit test for {@link IpNetwork}.
 */
public class IpNetworkTest extends TestBase {
    /**
     * Pseudo random number generator.
     */
    private Random  random = new Random();

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

    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link IpNetwork#create(InetAddress)}</li>
     *   <li>{@link IpNetwork#create(InetAddress, int)}</li>
     *   <li>{@link IpNetwork#create(IpPrefix)}</li>
     *   <li>{@link IpNetwork#create(Address)}</li>
     *   <li>{@link IpNetwork#create(String)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreate() throws Exception {
        int[] invalidPrefix = {
            Integer.MIN_VALUE, -10, -3, -2, -1,
            33, 34, 50, 100, 200000, Integer.MAX_VALUE,
        };

        for (int loop = 0; loop < 50; loop++) {
            int baseAddr = random.nextInt();
            InetAddress iaddr =
                InetAddress.getByAddress(NumberUtils.toBytes(baseAddr));
            String host = iaddr.getHostAddress();
            for (int i = 0; i <= 32; i++) {
                IpNetwork ip;
                int mask = Ip4Network.getNetMask(i);
                int addr = (baseAddr & mask);
                Ip4Network expected = new Ip4Network(addr, i);
                int prefix = i;
                if (i == 0) {
                    ip = IpNetwork.create(iaddr);
                    prefix = 32;
                } else {
                    ip = IpNetwork.create(iaddr, prefix);
                }
                assertTrue(ip instanceof Ip4Network);
                assertEquals(expected, ip);
                assertEquals(prefix, ip.getPrefixLength());

                String cidr = host + "/" + i;
                Ipv4Prefix ipv4 = new Ipv4Prefix(cidr);
                IpPrefix ipp = new IpPrefix(ipv4);
                ip = IpNetwork.create(ipp);
                assertTrue(ip instanceof Ip4Network);
                assertEquals(expected, ip);
                assertEquals(prefix, ip.getPrefixLength());

                Ipv4 i4 = new Ipv4Builder().setIpv4Address(ipv4).build();
                ip = IpNetwork.create(ipp);
                assertTrue(ip instanceof Ip4Network);
                assertEquals(expected, ip);
                assertEquals(prefix, ip.getPrefixLength());

                if (i == 0) {
                    cidr = host;
                } else {
                    cidr = host + "/" + i;
                }
                ip = IpNetwork.create(cidr);
                assertTrue(ip instanceof Ip4Network);
                assertEquals(expected, ip);
                assertEquals(prefix, ip.getPrefixLength());
            }

            // Specify invalid prefix length.
            for (int prefix: invalidPrefix) {
                try {
                    IpNetwork.create(iaddr, prefix);
                    unexpected();
                } catch (IllegalArgumentException e) {
                    String msg = "Invalid prefix length: " + prefix;
                    assertEquals(msg, e.getMessage());
                }

                String cidr = host + "/" + prefix;
                try {
                    IpNetwork.create(cidr);
                    unexpected();
                } catch (IllegalArgumentException e) {
                    String msg = "Invalid prefix length: " + prefix;
                    assertEquals(msg, e.getMessage());
                }
            }
        }

        InetAddress iaddr = null;
        InetAddress i6addr = InetAddress.getByName("::1");
        String i6host = i6addr.getHostAddress();
        String i6msg = "Unsupported IP address: " + i6addr;
        assertEquals(null, IpNetwork.create(iaddr));
        try {
            IpNetwork.create(i6addr);
            unexpected();
        } catch (IllegalArgumentException e) {
            assertEquals(i6msg, e.getMessage());
        }
        try {
            IpNetwork.create(i6host);
            unexpected();
        } catch (IllegalArgumentException e) {
            assertEquals(i6msg, e.getMessage());
        }

        for (int i = 0; i <= 32; i++) {
            assertEquals(null, IpNetwork.create(iaddr, i));
            try {
                IpNetwork.create(i6addr, i);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals(i6msg, e.getMessage());
            }

            String cidr = i6host + "/" + i;
            try {
                IpNetwork.create(cidr);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals(i6msg, e.getMessage());
            }
        }

        assertEquals(null, IpNetwork.create((IpPrefix)null));
        assertEquals(null, IpNetwork.create((String)null));
        assertEquals(null, IpNetwork.create((Address)null));
        assertEquals(null, IpNetwork.create((Ipv4)null));
        assertEquals(null, IpNetwork.create((Ipv6)null));

        Ipv6Prefix ipv6 = new Ipv6Prefix("::1/64");
        IpPrefix ipp = new IpPrefix(ipv6);
        try {
            IpNetwork.create(ipp);
            unexpected();
        } catch (IllegalArgumentException e) {
            String msg = "Unsupported IP prefix: " + ipp;
            assertEquals(msg, e.getMessage());
        }

        Ipv6 i6 = new Ipv6Builder().setIpv6Address(ipv6).build();
        try {
            IpNetwork.create(i6);
            unexpected();
        } catch (IllegalArgumentException e) {
            String msg = "Unsupported IP address: " + i6;
            assertEquals(msg, e.getMessage());
        }

        // Empty IP address.
        String[] emptyAddr = {"", "/1", "/32"};
        for (String cidr: emptyAddr) {
            try {
                IpNetwork.create(cidr);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("IP address cannot be empty.", e.getMessage());
            }
        }

        // Invalid IP address in CIDR notation.
        String[] badAddr = {
            "100.200.300.400",
            "100.200.300.400/15",
            "::1::2:::3/34",
        };
        for (String cidr: badAddr) {
            try {
                IpNetwork.create(cidr);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertTrue(e.getMessage().startsWith("Invalid IP address: "));
            }
        }

        // Invalid CIDR prefix length.
        String[] badPrefix = {
            "1.2.3.4/",
            "1.2.3.4/5/6",
            "1.2.3.4/prefix",
            "1.2.3.4/55555555555555555555555555555555555555555555",
        };
        for (String cidr: badPrefix) {
            try {
                IpNetwork.create(cidr);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertTrue(e.getMessage().startsWith("Invalid CIDR prefix: "));
            }
        }
    }
}
