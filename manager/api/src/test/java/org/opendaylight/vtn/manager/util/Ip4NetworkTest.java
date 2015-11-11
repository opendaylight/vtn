/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import java.io.ByteArrayInputStream;
import java.net.Inet4Address;
import java.net.InetAddress;
import java.util.HashSet;
import java.util.Random;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;
import org.opendaylight.vtn.manager.XmlDataType;
import org.opendaylight.vtn.manager.XmlValueType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv4;
import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.address.address.Ipv4Builder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpAddress;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpPrefix;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Address;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Prefix;

/**
 * JUnit test for {@link Ip4Network}.
 */
public class Ip4NetworkTest extends TestBase {
    /**
     * Root XML element name associated with {@link Ip4Network} class.
     */
    private static final String  XML_ROOT = "ip4-network";

    /**
     * Root XML element name associated with {@link IpNetwork} class.
     */
    private static final String  XML_ROOT_BASE = "ip-network";

    /**
     * Pseudo random number generator.
     */
    private Random  random = new Random();

    /**
     * Verify static constants.
     */
    @Test
    public void testConstants() {
        assertEquals(4, Ip4Network.SIZE);
    }

    /**
     * Test case for netmask utilities.
     *
     * <ul>
     *   <li>{@link Ip4Network#getPrefixLength(byte[])}</li>
     *   <li>{@link Ip4Network#getNetMask(int)}</li>
     *   <li>{@link Ip4Network#getInetMask(int)}</li>
     *   <li>{@link Ip4Network#getNetworkAddress(InetAddress, int)}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testNetMask() throws Exception {
        int[] masks = {
            (int)0xffffffff,
            (int)0x80000000,
            (int)0xc0000000,
            (int)0xe0000000,
            (int)0xf0000000,
            (int)0xf8000000,
            (int)0xfc000000,
            (int)0xfe000000,
            (int)0xff000000,
            (int)0xff800000,
            (int)0xffc00000,
            (int)0xffe00000,
            (int)0xfff00000,
            (int)0xfff80000,
            (int)0xfffc0000,
            (int)0xfffe0000,
            (int)0xffff0000,
            (int)0xffff8000,
            (int)0xffffc000,
            (int)0xffffe000,
            (int)0xfffff000,
            (int)0xfffff800,
            (int)0xfffffc00,
            (int)0xfffffe00,
            (int)0xffffff00,
            (int)0xffffff80,
            (int)0xffffffc0,
            (int)0xffffffe0,
            (int)0xfffffff0,
            (int)0xfffffff8,
            (int)0xfffffffc,
            (int)0xfffffffe,
            (int)0xffffffff,
        };

        InetAddress allSet = InetAddress.getByName("255.255.255.255");
        for (int i = 0; i < masks.length; i++) {
            int mask = masks[i];
            int prefix = (i == 0) ? 32 : i;
            byte[] bytes = NumberUtils.toBytes(mask);
            assertEquals(prefix, Ip4Network.getPrefixLength(bytes));
            assertEquals(mask, Ip4Network.getNetMask(i));

            InetAddress imask = Ip4Network.getInetMask(i);
            assertTrue(imask instanceof Inet4Address);
            assertArrayEquals(bytes, imask.getAddress());
            assertEquals(imask, Ip4Network.getNetworkAddress(allSet, i));
        }

        try {
            Ip4Network.getPrefixLength((byte[])null);
            unexpected();
        } catch (NullPointerException e) {
        }

        try {
            Ip4Network.getNetworkAddress((InetAddress)null, 1);
            unexpected();
        } catch (NullPointerException e) {
        }

        InetAddress v6addr = InetAddress.getByName("::1");
        try {
            Ip4Network.getNetworkAddress(v6addr, 1);
            unexpected();
        } catch (IllegalArgumentException e) {
            assertEquals("Invalid byte array length: 16", e.getMessage());
        }

        // Invalid array length.
        for (int i = -10; i <= 20; i++) {
            if (i == 4) {
                continue;
            }

            if (i >= 0) {
                byte[] bytes = new byte[i];
                try {
                    Ip4Network.getPrefixLength(bytes);
                } catch (IllegalArgumentException e) {
                    assertEquals("Invalid byte array length: " + i,
                                 e.getMessage());
                }
            } else {
                try {
                    Ip4Network.getNetMask(i);
                    unexpected();
                } catch (IllegalArgumentException e) {
                    assertEquals("Invalid prefix length: " + i,
                                 e.getMessage());
                }

                try {
                    Ip4Network.getInetMask(i);
                    unexpected();
                } catch (IllegalArgumentException e) {
                    assertEquals("Invalid prefix length: " + i,
                                 e.getMessage());
                }

                try {
                    Ip4Network.getNetworkAddress(allSet, i);
                    unexpected();
                } catch (IllegalArgumentException e) {
                    assertEquals("Invalid prefix length: " + i,
                                 e.getMessage());
                }
            }
        }

        for (int i = 33; i <= 50; i++) {
            try {
                Ip4Network.getNetMask(i);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid prefix length: " + i, e.getMessage());
            }

            try {
                Ip4Network.getInetMask(i);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid prefix length: " + i, e.getMessage());
            }

            try {
                Ip4Network.getNetworkAddress(allSet, i);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid prefix length: " + i, e.getMessage());
            }
        }

        // Invalid netmask.
        int[] basMask = {
            (int)0x81000000,
            (int)0x80100000,
            (int)0x80010000,
            (int)0x80001000,
            (int)0x80000100,
            (int)0x80000010,
            (int)0x80000001,
            (int)0xa0000000,
            (int)0x90000000,
            (int)0x08000000,
            (int)0x00010000,
            (int)0x00ffffff,
            (int)0xabcdef00,
            (int)0x7fffffff,
            (int)0x00000001,
            (int)0x0000000f,
            (int)0x000000f0,
            (int)0x00000f00,
            (int)0x0000f000,
            (int)0x000f0000,
            (int)0x00f00000,
            (int)0x0f000000,
        };

        for (int mask: basMask) {
            byte[] bytes = NumberUtils.toBytes(mask);
            try {
                Ip4Network.getPrefixLength(bytes);
                unexpected();
            } catch (IllegalArgumentException e) {
                String msg = "Invalid IPv4 netmask: " +
                    Integer.toHexString(mask);
                assertEquals(msg, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link Ip4Network#getInetAddress(int)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetInetAddress() {
        for (int i = 0; i < 30; i++) {
            int addr = random.nextInt();
            byte[] bytes = NumberUtils.toBytes(addr);
            InetAddress iaddr = Ip4Network.getInetAddress(addr);
            assertTrue(iaddr instanceof Inet4Address);
            assertArrayEquals(bytes, iaddr.getAddress());
        }
    }

    /**
     * Test case for {@link Ip4Network#create(Ipv4Address)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCreateIpv4Address() throws Exception {
        assertEquals(null, Ip4Network.create((Ipv4Address)null));

        String[] taddrs = {
            "0.0.0.0",
            "127.0.0.1",
            "1.2.3.4",
            "10.20.30.40",
            "192.168.45.23",
            "255.255.255.255",
        };

        for (String text: taddrs) {
            InetAddress iaddr = InetAddress.getByName(text);
            Ipv4Address ipv4 = new Ipv4Address(text);
            Ip4Network ip4 = Ip4Network.create(ipv4);
            assertEquals(iaddr, ip4.getInetAddress());
            assertEquals(ipv4, ip4.getIpAddress().getIpv4Address());

            // Zone should be ignored.
            String text1 = text + "%1";
            Ip4Network newIp4 = Ip4Network.create(new Ipv4Address(text1));
            assertEquals(ip4, newIp4);
            assertEquals(iaddr, newIp4.getInetAddress());
            assertEquals(ipv4, newIp4.getIpAddress().getIpv4Address());
        }
    }

    /**
     * Test case for constructors and getter methods.
     *
     * <ul>
     *   <li>{@link Ip4Network#toIp4Address(IpNetwork)}</li>
     *   <li>{@link Ip4Network#Ip4Network(int)}</li>
     *   <li>{@link Ip4Network#Ip4Network(int, int)}</li>
     *   <li>{@link Ip4Network#Ip4Network(byte[])}</li>
     *   <li>{@link Ip4Network#Ip4Network(byte[], int)}</li>
     *   <li>{@link Ip4Network#Ip4Network(InetAddress)}</li>
     *   <li>{@link Ip4Network#Ip4Network(InetAddress, int)}</li>
     *   <li>{@link Ip4Network#Ip4Network(String)}</li>
     *   <li>{@link Ip4Network#getAddress()}</li>
     *   <li>{@link Ip4Network#getNetMask()}</li>
     *   <li>{@link Ip4Network#getMaxPrefix()}</li>
     *   <li>{@link Ip4Network#getIpPrefix()}</li>
     *   <li>{@link Ip4Network#getIpAddress()}</li>
     *   <li>{@link Ip4Network#getMdAddress()}</li>
     *   <li>{@link Ip4Network#getCidrText()}</li>
     *   <li>{@link Ip4Network#getHostAddress()}</li>
     *   <li>{@link Ip4Network#getBytes()}</li>
     *   <li>{@link IpNetwork#getPrefixLength()}</li>
     *   <li>{@link IpNetwork#getInetAddress()}</li>
     *   <li>{@link IpNetwork#getText()}</li>
     *   <li>{@link IpNetwork#isAddress()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetter() throws Exception {
        for (int loop = 0; loop < 50; loop++) {
            int addr = random.nextInt();
            int prefix = 32;
            int netmask = -1;
            byte[] bytes = NumberUtils.toBytes(addr);
            InetAddress iaddr = InetAddress.getByAddress(bytes);
            String host = iaddr.getHostAddress();
            String text = host;
            String cidr = host + "/32";
            Ipv4Prefix ipv4 = new Ipv4Prefix(cidr);
            IpPrefix ipp = new IpPrefix(ipv4);
            IpAddress ipa = new IpAddress(new Ipv4Address(host));
            Ipv4 i4 = new Ipv4Builder().setIpv4Address(ipv4).build();

            Ip4Network[] cases = {
                new Ip4Network(addr),
                new Ip4Network(bytes),
                new Ip4Network(iaddr),
                new Ip4Network(host),
                new Ip4Network(cidr),
            };
            for (Ip4Network ip4: cases) {
                assertEquals(addr, ip4.getAddress());
                assertEquals(netmask, ip4.getNetMask());
                assertEquals(32, ip4.getMaxPrefix());
                IpPrefix ipp1 = ip4.getIpPrefix();
                assertEquals(ipp, ipp1);
                assertSame(ipp1, ip4.getIpPrefix());
                assertEquals(ipa, ip4.getIpAddress());
                assertEquals(i4, ip4.getMdAddress());
                String cidr1 = ip4.getCidrText();
                assertEquals(cidr, cidr1);
                assertSame(cidr1, ip4.getCidrText());
                String host1 = ip4.getHostAddress();
                assertEquals(host, host1);
                assertSame(host1, ip4.getHostAddress());
                byte[] bytes1 = ip4.getBytes();
                assertArrayEquals(bytes, bytes1);
                assertNotSame(bytes1, ip4.getBytes());
                assertEquals(prefix, ip4.getPrefixLength());
                InetAddress iaddr1 = ip4.getInetAddress();
                assertEquals(iaddr, iaddr1);
                assertSame(iaddr1, ip4.getInetAddress());
                String text1 = ip4.getText();
                assertEquals(text, text1);
                assertSame(text1, ip4.getText());
                assertEquals(true, ip4.isAddress());
                assertSame(ip4, Ip4Network.toIp4Address(ip4));
            }

            for (int i = 0; i <= 32; i++) {
                prefix = (i == 0) ? 32 : i;
                netmask = Ip4Network.getNetMask(i);
                int maskedAddr = addr & netmask;
                byte[] maskedBytes = NumberUtils.toBytes(maskedAddr);
                InetAddress maskedInet = InetAddress.getByAddress(maskedBytes);
                String maskedHost = maskedInet.getHostAddress();
                String maskedCidr = maskedHost + "/" + prefix;
                String maskedText = (prefix == 32)
                    ? maskedHost : maskedCidr;
                Ipv4Prefix maskedIpv4 = new Ipv4Prefix(maskedCidr);
                IpPrefix maskedIpp = new IpPrefix(maskedIpv4);
                IpAddress maskedIpa =
                    new IpAddress(new Ipv4Address(maskedHost));
                Ipv4 maskedI4 = new Ipv4Builder().setIpv4Address(maskedIpv4).
                    build();
                String arg1 = maskedHost + "/" + i;
                String arg2 = host + "/" + i;

                cases = new Ip4Network[]{
                    new Ip4Network(addr, i),
                    new Ip4Network(bytes, i),
                    new Ip4Network(iaddr, i),
                    new Ip4Network(arg1),
                    new Ip4Network(arg2),
                };
                for (Ip4Network ip4: cases) {
                    assertEquals(maskedAddr, ip4.getAddress());
                    assertEquals(netmask, ip4.getNetMask());
                    assertEquals(32, ip4.getMaxPrefix());
                    IpPrefix ipp1 = ip4.getIpPrefix();
                    assertEquals(maskedIpp, ipp1);
                    assertSame(ipp1, ip4.getIpPrefix());
                    assertEquals(maskedIpa, ip4.getIpAddress());
                    assertEquals(maskedI4, ip4.getMdAddress());
                    String cidr1 = ip4.getCidrText();
                    assertEquals(maskedCidr, cidr1);
                    assertSame(cidr1, ip4.getCidrText());
                    String host1 = ip4.getHostAddress();
                    assertEquals(maskedHost, host1);
                    assertSame(host1, ip4.getHostAddress());
                    byte[] bytes1 = ip4.getBytes();
                    assertArrayEquals(maskedBytes, bytes1);
                    assertNotSame(bytes1, ip4.getBytes());
                    assertEquals(prefix, ip4.getPrefixLength());
                    InetAddress iaddr1 = ip4.getInetAddress();
                    assertEquals(maskedInet, iaddr1);
                    assertSame(iaddr1, ip4.getInetAddress());
                    String text1 = ip4.getText();
                    assertEquals(maskedText, text1);
                    assertSame(text1, ip4.getText());
                    if (prefix == 32) {
                        assertEquals(true, ip4.isAddress());
                        assertSame(ip4, Ip4Network.toIp4Address(ip4));
                    } else {
                        assertEquals(false, ip4.isAddress());
                        assertEquals(null, Ip4Network.toIp4Address(ip4));
                    }
                }
            }
        }

        // Null argument.
        try {
            new Ip4Network((byte[])null);
            unexpected();
        } catch (NullPointerException e) {
        }

        try {
            new Ip4Network((InetAddress)null);
            unexpected();
        } catch (NullPointerException e) {
        }

        try {
            new Ip4Network((String)null);
            unexpected();
        } catch (NullPointerException e) {
        }

        IpNetwork[] invalidNet = {
            null,
            new UnknownIpNetwork(),
        };
        for (IpNetwork ipn: invalidNet) {
            assertEquals(null, Ip4Network.toIp4Address(ipn));
        }

        // Invalid prefix length.
        InetAddress v4addr = InetAddress.getByName("127.0.0.1");
        int badValue = -10;
        do {
            String msg = "Invalid prefix length: " + badValue;
            try {
                new Ip4Network(0, badValue);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals(msg, e.getMessage());
            }

            try {
                new Ip4Network(v4addr.getAddress(), badValue);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals(msg, e.getMessage());
            }

            try {
                new Ip4Network(v4addr, badValue);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals(msg, e.getMessage());
            }

            String cidr = "127.0.0.1/" + badValue;
            try {
                new Ip4Network(cidr);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals(msg, e.getMessage());
            }

            badValue++;
            if (badValue == 0) {
                badValue = 33;
            }
        } while (badValue <= 50);

        // Invalid byte array length.
        badValue = 0;
        do {
            String msg = "Invalid byte array length: " + badValue;
            byte[] bytes = new byte[badValue];
            try {
                new Ip4Network(bytes);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals(msg, e.getMessage());
            }

            badValue++;
            if (badValue == 4) {
                badValue++;
            }
        } while (badValue <= 30);

        // Invalid IP address.
        InetAddress v6addr = InetAddress.getByName("::1");
        try {
            new Ip4Network(v6addr);
            unexpected();
        } catch (IllegalArgumentException e) {
            assertEquals("Invalid byte array length: 16", e.getMessage());
        }

        String[] badAddr = {
            "::1",
            "::1/25",
            "2048::ffab:cdef/65",
        };
        for (String bad: badAddr) {
            try {
                new Ip4Network(bad);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid byte array length: 16", e.getMessage());
            }
        }

        // Invalid CIDR notation.
        badAddr = new String[]{
            "10.1.2.3/",
            "10.1.2.3/0x123",
            "10.1.2.3/3.141592",
            "10.1.2.3/1/2/3",
        };
        for (String bad: badAddr) {
            try {
                new Ip4Network(bad);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid CIDR prefix: " + bad, e.getMessage());
            }
        }

        badAddr = new String[]{
            "Bad IP address",
            "123.456.789.abc",
            "abc::ddee::123",
        };
        for (String bad: badAddr) {
            try {
                new Ip4Network(bad);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid IP address: " + bad, e.getMessage());
            }

            try {
                new Ip4Network(bad + "/8");
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid IP address: " + bad, e.getMessage());
            }
        }
    }

    /**
     * Test case for {@link Ip4Network#contains(IpNetwork)}.
     */
    @Test
    public void testContains() {
        for (int loop = 0; loop < 50; loop++) {
            int addr = random.nextInt();
            for (int prefix = 1; prefix <= 32; prefix++) {
                Ip4Network ip4 = new Ip4Network(addr, prefix);
                assertEquals(true, ip4.contains(ip4));
                for (int bit = 1; bit <= prefix; bit++) {
                    int mask = (1 << (32 - bit));
                    Ip4Network nw = new Ip4Network(addr ^ mask);
                    assertEquals(false, ip4.contains(nw));
                }
                for (int bit = prefix + 1; bit <= 32; bit++) {
                    int mask = (1 << (32 - bit));
                    Ip4Network nw = new Ip4Network(addr ^ mask);
                    assertEquals(true, ip4.contains(nw));
                }
            }
        }

        Ip4Network ip4 = new Ip4Network(0);
        try {
            ip4.contains(null);
            unexpected();
        } catch (NullPointerException e) {
        }

        UnknownIpNetwork unknown = new UnknownIpNetwork();
        try {
            ip4.contains(unknown);
            unexpected();
        } catch (IllegalArgumentException e) {
            assertEquals("Unexpected IpNetwork: " + unknown, e.getMessage());
        }
    }

    /**
     * Test case for {@link Ip4Network#equals(Object)} and
     * {@link Ip4Network#hashCode()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testEquals() throws Exception {
        HashSet<Integer> addrSet = new HashSet<>();
        assertEquals(true, addrSet.add(0));
        assertEquals(true, addrSet.add(-1));
        do {
            int addr = random.nextInt();
            addrSet.add(addr);
        } while (addrSet.size() < 30);

        HashSet<Long> tested = new HashSet<>();
        HashSet<Object> set = new HashSet<>();
        int count = 0;
        for (Integer i: addrSet) {
            int addr = i.intValue();
            byte[] bytes = NumberUtils.toBytes(addr);
            InetAddress iaddr = InetAddress.getByAddress(bytes);
            String host = iaddr.getHostAddress();
            for (int prefix = 1; prefix <= 32; prefix++) {
                Ip4Network nw1 = new Ip4Network(addr, prefix);

                // Determine whether this network address is already tested.
                int mask = Ip4Network.getNetMask(prefix);
                long maskedAddr = (long)(addr & mask);
                long lv = (maskedAddr << 32) | ((long)mask & 0xffffffffL);
                if (tested.add(lv)) {
                    // This network address is not yet tested.
                    Ip4Network nw2 = new Ip4Network(addr, prefix);
                    testEquals(set, nw1, nw2);

                    // Ensure that the object identity is never changed even if
                    // attributes are cached.
                    nw2.getBytes();
                    nw2.getInetAddress();
                    nw2.getCidrText();
                    assertEquals(false, set.add(nw2));
                    count++;
                } else {
                    assertEquals(false, set.add(nw1));
                }
            }

            // Ensure that the zero prefix is treated as 32.
            assertEquals(false, set.add(new Ip4Network(addr, 0)));
            assertEquals(false, set.add(new Ip4Network(bytes, 0)));
            assertEquals(false, set.add(new Ip4Network(iaddr, 0)));
            assertEquals(false, set.add(new Ip4Network(host)));
            assertEquals(false, set.add(new Ip4Network(host + "/0")));
        }

        assertEquals(count, set.size());
    }

    /**
     * Test case for {@link Ip4Network#toString()}.
     */
    @Test
    public void testToString() {
        for (int loop = 0; loop < 30; loop++) {
            int addr = random.nextInt();
            for (int prefix = 1; prefix <= 32; prefix++) {
                int mask = Ip4Network.getNetMask(prefix);
                int maskedAddr = addr & mask;
                String host = String.format(
                    "%d.%d.%d.%d", (maskedAddr >>> 24) & 0xff,
                    (maskedAddr >>> 16) & 0xff,
                    (maskedAddr >>> 8) & 0xff, maskedAddr & 0xff);
                String text = (prefix == 32) ? host : host + "/" + prefix;
                String expected = String.format("Ip4Network[%s]", text);
                Ip4Network ip4 = new Ip4Network(addr, prefix);
                assertEquals(expected, ip4.toString());
            }
        }
    }

    /**
     * Ensure that {@link Ip4Network} is serializable.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSerialize() throws Exception {
        for (int loop = 0; loop < 20; loop++) {
            int addr = random.nextInt();
            for (int prefix = 1; prefix <= 32; prefix++) {
                int mask = Ip4Network.getNetMask(prefix);
                int maskedAddr = addr & mask;
                byte[] bytes = NumberUtils.toBytes(maskedAddr);
                InetAddress iaddr = InetAddress.getByAddress(bytes);
                String host = String.format(
                    "%d.%d.%d.%d", (maskedAddr >>> 24) & 0xff,
                    (maskedAddr >>> 16) & 0xff,
                    (maskedAddr >>> 8) & 0xff, maskedAddr & 0xff);
                String cidr = host + "/" + prefix;
                String text = (prefix == 32) ? host : cidr;
                Ip4Network nw = new Ip4Network(addr, prefix);
                Ip4Network nw1 = serializeTest(nw, Ip4Network.class);
                assertEquals(maskedAddr, nw1.getAddress());
                assertEquals(mask, nw1.getNetMask());
                assertArrayEquals(bytes, nw1.getBytes());
                assertEquals(iaddr, nw1.getInetAddress());
                assertEquals(host, nw1.getHostAddress());
                assertEquals(cidr, nw1.getCidrText());
                assertEquals(text, nw1.getText());

                Ip4Network nw2 = serializeTest(nw1, Ip4Network.class);
                assertEquals(maskedAddr, nw2.getAddress());
                assertEquals(mask, nw2.getNetMask());
                assertArrayEquals(bytes, nw2.getBytes());
                assertEquals(iaddr, nw2.getInetAddress());
                assertEquals(host, nw2.getHostAddress());
                assertEquals(cidr, nw2.getCidrText());
                assertEquals(text, nw2.getText());
            }
        }
    }

    /**
     * Ensure that {@link Ip4Network} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        int[] prefixes = {
            1, 8, 25, 32,
        };
        for (int loop = 0; loop < 20; loop++) {
            int addr = random.nextInt();
            for (int prefix: prefixes) {
                int mask = Ip4Network.getNetMask(prefix);
                int maskedAddr = addr & mask;
                byte[] bytes = NumberUtils.toBytes(maskedAddr);
                InetAddress iaddr = InetAddress.getByAddress(bytes);
                String host = String.format(
                    "%d.%d.%d.%d", (maskedAddr >>> 24) & 0xff,
                    (maskedAddr >>> 16) & 0xff,
                    (maskedAddr >>> 8) & 0xff, maskedAddr & 0xff);
                String cidr = host + "/" + prefix;
                String text = (prefix == 32) ? host : cidr;
                Ip4Network nw = new Ip4Network(addr, prefix);
                Ip4Network nw1 = jaxbTest(nw, Ip4Network.class, XML_ROOT);
                assertEquals(maskedAddr, nw1.getAddress());
                assertEquals(mask, nw1.getNetMask());
                assertArrayEquals(bytes, nw1.getBytes());
                assertEquals(iaddr, nw1.getInetAddress());
                assertEquals(host, nw1.getHostAddress());
                assertEquals(cidr, nw1.getCidrText());
                assertEquals(text, nw1.getText());

                Ip4Network nw2 = jaxbTest(nw1, Ip4Network.class, XML_ROOT);
                assertEquals(maskedAddr, nw2.getAddress());
                assertEquals(mask, nw2.getNetMask());
                assertArrayEquals(bytes, nw2.getBytes());
                assertEquals(iaddr, nw2.getInetAddress());
                assertEquals(host, nw2.getHostAddress());
                assertEquals(cidr, nw2.getCidrText());
                assertEquals(text, nw2.getText());

                // Ensure that Ip4Network can be unmarshalled via IpNetwork.
                IpNetwork n = jaxbTest(nw2, IpNetwork.class, XML_ROOT_BASE);
                assertTrue(n instanceof Ip4Network);
                Ip4Network nw3 = (Ip4Network)n;
                assertEquals(maskedAddr, nw3.getAddress());
                assertEquals(mask, nw3.getNetMask());
                assertArrayEquals(bytes, nw3.getBytes());
                assertEquals(iaddr, nw3.getInetAddress());
                assertEquals(host, nw3.getHostAddress());
                assertEquals(cidr, nw3.getCidrText());
                assertEquals(text, nw3.getText());
            }
        }

        // Ensure that leading and trailing whitespaces are eliminated.
        String cidr = "12.34.56.78/29";
        String host = "12.34.56.72";
        int maskedAddr = 0x0c223848;
        StringBuilder builder = new StringBuilder(XML_DECLARATION);
        builder.append('<').append(XML_ROOT).append('>').
            append("\n    \t\t\r    ").append(cidr).
            append("     \t\t\t\t     \n     \r\n").
            append("</").append(XML_ROOT).append('>');
        String xml = builder.toString();

        ByteArrayInputStream in = new ByteArrayInputStream(xml.getBytes());
        Unmarshaller um = createUnmarshaller(Ip4Network.class);
        Object o = um.unmarshal(in);
        assertTrue(o instanceof Ip4Network);
        Ip4Network ip4 = (Ip4Network)o;
        assertEquals(maskedAddr, ip4.getAddress());
        assertEquals(host, ip4.getHostAddress());
        assertEquals(29, ip4.getPrefixLength());

        // Ensure that broken values in XML can be detected.
        XmlDataType dtype = new XmlValueType(XML_ROOT, Ip4Network.class);
        jaxbErrorTest(um, Ip4Network.class, dtype);
        jaxbErrorTest(createUnmarshaller(IpNetwork.class), Ip4Network.class,
                      dtype);
    }
}
