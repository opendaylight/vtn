/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import java.io.ByteArrayInputStream;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Locale;
import java.util.Map;
import java.util.Random;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.TestBase;
import org.opendaylight.vtn.manager.XmlValueType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetDestination;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetDestinationBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetSource;
import org.opendaylight.yang.gen.v1.urn.opendaylight.model.match.types.rev131026.ethernet.match.fields.EthernetSourceBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * JUnit test for {@link EtherAddress}.
 */
public class EtherAddressTest extends TestBase {
    /**
     * Root XML element name associated with {@link EtherAddress} class.
     */
    private static final String  XML_ROOT = "ether-address";

    /**
     * Pseudo random number generator.
     */
    private Random  random = new Random();

    /**
     * Verify static constants.
     */
    @Test
    public void testConstants() {
        assertEquals(6, EtherAddress.SIZE);
        assertEquals(1, EtherAddress.BIT_MULTICAST);
        assertEquals(0x010000000000L, EtherAddress.MASK_MULTICAST);
        assertEquals(0xffffffffffffL, EtherAddress.BROADCAST.getAddress());
    }

    /**
     * Test case for {@link EtherAddress#toLong(byte[] b)} and
     * {@link EtherAddress#toBytes(long)}.
     */
    @Test
    public void testByteArray() {
        try {
            EtherAddress.toLong((byte[])null);
            unexpected();
        } catch (NullPointerException e) {
        }

        for (int i = 0; i <= 10; i++) {
            if (i == 6) {
                continue;
            }

            byte[] b = new byte[i];
            try {
                EtherAddress.toLong(b);
                unexpected();
            } catch (IllegalArgumentException e) {
                assertEquals("Invalid byte array length: " + i,
                             e.getMessage());
            }
        }

        HashMap<Long, byte[]> cases = new HashMap<>();
        cases.put(0L,
                  new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                             (byte)0x00, (byte)0x00, (byte)0x00});
        cases.put(1L,
                  new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                             (byte)0x00, (byte)0x00, (byte)0x01});
        cases.put(0xffL,
                  new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                             (byte)0x00, (byte)0x00, (byte)0xff});
        cases.put(0x310fcL,
                  new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                             (byte)0x03, (byte)0x10, (byte)0xfc});
        cases.put(0x123456L,
                  new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                             (byte)0x12, (byte)0x34, (byte)0x56});
        cases.put(0xaabbccddeeffL,
                  new byte[]{(byte)0xaa, (byte)0xbb, (byte)0xcc,
                             (byte)0xdd, (byte)0xee, (byte)0xff});
        cases.put(0xffffff123456L,
                  new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                             (byte)0x12, (byte)0x34, (byte)0x56});
        cases.put(0xfffffffffffeL,
                  new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                             (byte)0xff, (byte)0xff, (byte)0xfe});
        cases.put(0xffffffffffffL,
                  new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                             (byte)0xff, (byte)0xff, (byte)0xff});

        for (Map.Entry<Long, byte[]> entry: cases.entrySet()) {
            long addr = entry.getKey().longValue();
            byte[] bytes = entry.getValue();
            assertEquals(addr, EtherAddress.toLong(bytes));
            assertArrayEquals(bytes, EtherAddress.toBytes(addr));
        }
    }

    /**
     * Test case for {@link EtherAddress#toLong(String)} and
     * {@link EtherAddress#toBytes(String)}.
     */
    @Test
    public void testHexString() {
        try {
            EtherAddress.toLong((String)null);
            unexpected();
        } catch (NullPointerException e) {
        }

        assertEquals(null, EtherAddress.toBytes((String)null));

        HashMap<String, byte[]> cases = new HashMap<>();
        cases.put("00:00:00:00:00:00",
                  new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                             (byte)0x00, (byte)0x00, (byte)0x00});
        cases.put("00:00:00:00:00:01",
                  new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                             (byte)0x00, (byte)0x00, (byte)0x01});
        cases.put("00:00:00:00:00:ff",
                  new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                             (byte)0x00, (byte)0x00, (byte)0xff});
        cases.put("00:00:00:03:10:fc",
                  new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                             (byte)0x03, (byte)0x10, (byte)0xfc});
        cases.put("00:00:00:12:34:56",
                  new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                             (byte)0x12, (byte)0x34, (byte)0x56});
        cases.put("aa:bb:cc:dd:ee:ff",
                  new byte[]{(byte)0xaa, (byte)0xbb, (byte)0xcc,
                             (byte)0xdd, (byte)0xee, (byte)0xff});
        cases.put("ff:ff:ff:12:34:56",
                  new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                             (byte)0x12, (byte)0x34, (byte)0x56});
        cases.put("ff:ff:ff:ff:ff:fe",
                  new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                             (byte)0xff, (byte)0xff, (byte)0xfe});
        cases.put("ff:ff:ff:ff:ff:ff",
                  new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                             (byte)0xff, (byte)0xff, (byte)0xff});

        for (Map.Entry<String, byte[]> entry: cases.entrySet()) {
            String hex = entry.getKey();
            byte[] bytes = entry.getValue();
            assertArrayEquals(bytes, EtherAddress.toBytes(hex));
            assertEquals(EtherAddress.toLong(bytes),
                         EtherAddress.toLong(hex));
        }
    }

    /**
     * Test case for {@link EtherAddress#isBroadcast(byte[])} and
     * {@link EtherAddress#isBroadcast()}.
     */
    @Test
    public void testIsBroadcast() {
        byte[][] testBytes = {
            new byte[0],
            new byte[]{(byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xff, (byte)0xff},
            new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                       (byte)0x00, (byte)0x00, (byte)0x00},
            new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                       (byte)0x00, (byte)0x00, (byte)0x01},
            new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                       (byte)0x00, (byte)0x00, (byte)0xff},
            new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                       (byte)0x03, (byte)0x10, (byte)0xfc},
            new byte[]{(byte)0x00, (byte)0x00, (byte)0x00,
                       (byte)0x12, (byte)0x34, (byte)0x56},
            new byte[]{(byte)0xaa, (byte)0xbb, (byte)0xcc,
                       (byte)0xdd, (byte)0xee, (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0x12, (byte)0x34, (byte)0x56},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xff, (byte)0xfe},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xef, (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xfe, (byte)0xff, (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xef,
                       (byte)0xff, (byte)0xff, (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xfe, (byte)0xff,
                       (byte)0xff, (byte)0xff, (byte)0xff},
            new byte[]{(byte)0xfe, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xff, (byte)0xff},
        };

        for (byte[] bytes: testBytes) {
            assertEquals(false, EtherAddress.isBroadcast(bytes));
            if (bytes.length == 6) {
                EtherAddress ea = new EtherAddress(bytes);
                assertEquals(false, ea.isBroadcast());
            }
        }

        byte[] broadcast = {
            (byte)0xff, (byte)0xff, (byte)0xff,
            (byte)0xff, (byte)0xff, (byte)0xff,
        };
        assertEquals(true, EtherAddress.isBroadcast(broadcast));
        EtherAddress ea = new EtherAddress(broadcast);
        assertEquals(true, ea.isBroadcast());
        ea = new EtherAddress(0xffffffffffffL);
        assertEquals(true, ea.isBroadcast());
        ea = new EtherAddress("ff:ff:ff:ff:ff:ff");
        assertEquals(true, ea.isBroadcast());
        ea = new EtherAddress(new MacAddress("FF:FF:FF:FF:FF:FF"));
        assertEquals(true, ea.isBroadcast());
    }

    /**
     * Test case for {@link EtherAddress#isUnicast(byte[])} and
     * {@link EtherAddress#isUnicast()}.
     */
    @Test
    public void testIsUnicast() {
        byte[][] testBytes = {
            new byte[0],
            new byte[]{(byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xff},
            new byte[]{(byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xff, (byte)0xff,
                       (byte)0xff, (byte)0xff, (byte)0xff},
        };

        for (byte[] bytes: testBytes) {
            assertEquals(false, EtherAddress.isUnicast(bytes));
        }

        assertEquals(false, EtherAddress.isUnicast(EtherAddress.BROADCAST.
                                                   getBytes()));
        assertEquals(false, EtherAddress.BROADCAST.isUnicast());

        byte[] bytes = new byte[6];
        for (int i = 0; i < 100; i++) {
            random.nextBytes(bytes);
            bytes[0] &= 0xfe;
            assertEquals(true, EtherAddress.isUnicast(bytes));
            EtherAddress ea = new EtherAddress(bytes);
            assertEquals(true, ea.isUnicast());

            bytes[0] |= 0x01;
            assertEquals(false, EtherAddress.isUnicast(bytes));
            ea = new EtherAddress(bytes);
            assertEquals(false, ea.isUnicast());
        }
    }

    /**
     * Test case for constructors and getter methods.
     *
     * <ul>
     *   <li>{@link EtherAddress#create(byte[])}</li>
     *   <li>{@link EtherAddress#create(String)}</li>
     *   <li>{@link EtherAddress#create(MacAddress)}</li>
     *   <li>{@link EtherAddress#create(MacAddressFilter)}</li>
     *   <li>{@link EtherAddress#EtherAddress(long)}</li>
     *   <li>{@link EtherAddress#EtherAddress(byte[])}</li>
     *   <li>{@link EtherAddress#EtherAddress(String)}</li>
     *   <li>{@link EtherAddress#EtherAddress(MacAddress)}</li>
     *   <li>{@link EtherAddress#getAddress()}</li>
     *   <li>{@link EtherAddress#getBytes()}</li>
     *   <li>{@link EtherAddress#getMacAddress()}</li>
     *   <li>{@link EtherAddress#getText()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetter() throws Exception {
        MacAddress mask = new MacAddress("ff:ff:ff:00:00:00");
        byte[] bytes = new byte[6];
        for (int i = 0; i < 100; i++) {
            random.nextBytes(bytes);
            long addr = EtherAddress.toLong(bytes);
            String hex = ByteUtils.toHexString(bytes);
            MacAddress mac = new MacAddress(hex);
            EtherAddress ea = new EtherAddress(bytes);
            assertEquals(addr, ea.getAddress());
            byte[] b = ea.getBytes();
            assertArrayEquals(bytes, b);
            assertNotSame(bytes, b);
            assertNotSame(b, ea.getBytes());
            MacAddress mac1 = ea.getMacAddress();
            assertEquals(mac, mac1);
            assertSame(mac1, ea.getMacAddress());
            assertEquals(hex, ea.getText());

            ea = new EtherAddress(addr);
            assertEquals(addr, ea.getAddress());
            b = ea.getBytes();
            assertArrayEquals(bytes, b);
            assertNotSame(bytes, b);
            assertNotSame(b, ea.getBytes());
            mac1 = ea.getMacAddress();
            assertEquals(mac, mac1);
            assertSame(mac1, ea.getMacAddress());
            assertEquals(hex, ea.getText());

            ea = new EtherAddress(hex);
            assertEquals(addr, ea.getAddress());
            b = ea.getBytes();
            assertArrayEquals(bytes, b);
            assertNotSame(bytes, b);
            assertNotSame(b, ea.getBytes());
            mac1 = ea.getMacAddress();
            assertEquals(mac, mac1);
            assertSame(mac1, ea.getMacAddress());
            assertEquals(hex, ea.getText());

            ea = EtherAddress.create(hex);
            assertEquals(addr, ea.getAddress());
            b = ea.getBytes();
            assertArrayEquals(bytes, b);
            assertNotSame(bytes, b);
            assertNotSame(b, ea.getBytes());
            mac1 = ea.getMacAddress();
            assertEquals(mac, mac1);
            assertSame(mac1, ea.getMacAddress());
            assertEquals(hex, ea.getText());

            ea = new EtherAddress(mac);
            assertEquals(addr, ea.getAddress());
            b = ea.getBytes();
            assertArrayEquals(bytes, b);
            assertNotSame(bytes, b);
            assertNotSame(b, ea.getBytes());
            assertSame(mac, ea.getMacAddress());
            assertEquals(hex, ea.getText());

            for (MacAddress m: new MacAddress[]{null, mask}) {
                EthernetSource src = new EthernetSourceBuilder().
                    setAddress(mac).setMask(m).build();
                ea = EtherAddress.create(src);
                assertEquals(addr, ea.getAddress());
                b = ea.getBytes();
                assertArrayEquals(bytes, b);
                assertNotSame(bytes, b);
                assertNotSame(b, ea.getBytes());
                assertSame(mac, ea.getMacAddress());
                assertEquals(hex, ea.getText());

                EthernetDestination dst = new EthernetDestinationBuilder().
                    setAddress(mac).setMask(m).build();
                ea = EtherAddress.create(src);
                assertEquals(addr, ea.getAddress());
                b = ea.getBytes();
                assertArrayEquals(bytes, b);
                assertNotSame(bytes, b);
                assertNotSame(b, ea.getBytes());
                assertSame(mac, ea.getMacAddress());
                assertEquals(hex, ea.getText());
            }

            ea = EtherAddress.create(mac);
            assertEquals(addr, ea.getAddress());
            b = ea.getBytes();
            assertArrayEquals(bytes, b);
            assertNotSame(bytes, b);
            assertNotSame(b, ea.getBytes());
            assertSame(mac, ea.getMacAddress());
            assertEquals(hex, ea.getText());

            String uhex = hex.toUpperCase(Locale.ENGLISH);
            if (!hex.equals(uhex)) {
                ea = new EtherAddress(uhex);
                assertEquals(addr, ea.getAddress());
                b = ea.getBytes();
                assertArrayEquals(bytes, b);
                assertNotSame(bytes, b);
                assertNotSame(b, ea.getBytes());
                mac1 = ea.getMacAddress();
                assertEquals(mac, mac1);
                assertSame(mac1, ea.getMacAddress());
                assertEquals(hex, ea.getText());

                MacAddress umac = new MacAddress(uhex);
                ea = new EtherAddress(umac);
                assertEquals(addr, ea.getAddress());
                b = ea.getBytes();
                assertArrayEquals(bytes, b);
                assertNotSame(bytes, b);
                assertNotSame(b, ea.getBytes());
                mac1 = ea.getMacAddress();
                assertEquals(mac, mac1);
                assertNotSame(umac, mac1);
                assertSame(mac1, ea.getMacAddress());
                assertEquals(hex, ea.getText());

                for (MacAddress m: new MacAddress[]{null, mask}) {
                    EthernetSource src = new EthernetSourceBuilder().
                        setAddress(umac).setMask(m).build();
                    ea = EtherAddress.create(src);
                    assertEquals(addr, ea.getAddress());
                    b = ea.getBytes();
                    assertArrayEquals(bytes, b);
                    assertNotSame(bytes, b);
                    assertNotSame(b, ea.getBytes());
                    mac1 = ea.getMacAddress();
                    assertEquals(mac, mac1);
                    assertNotSame(umac, mac1);
                    assertSame(mac1, ea.getMacAddress());
                    assertEquals(hex, ea.getText());

                    EthernetDestination dst = new EthernetDestinationBuilder().
                        setAddress(umac).setMask(m).build();
                    ea = EtherAddress.create(dst);
                    assertEquals(addr, ea.getAddress());
                    b = ea.getBytes();
                    assertArrayEquals(bytes, b);
                    assertNotSame(bytes, b);
                    assertNotSame(b, ea.getBytes());
                    mac1 = ea.getMacAddress();
                    assertEquals(mac, mac1);
                    assertNotSame(umac, mac1);
                    assertSame(mac1, ea.getMacAddress());
                    assertEquals(hex, ea.getText());
                }
            }
        }

        assertEquals(null, EtherAddress.create((byte[])null));
        assertEquals(null, EtherAddress.create((String)null));
        assertEquals(null, EtherAddress.create((MacAddress)null));
        assertEquals(null, EtherAddress.create((EthernetSource)null));
        assertEquals(null, EtherAddress.create((EthernetDestination)null));

        EthernetSource src = new EthernetSourceBuilder().setMask(mask).build();
        assertEquals(null, EtherAddress.create(src));
        EthernetDestination dst = new EthernetDestinationBuilder().
            setMask(mask).build();
        assertEquals(null, EtherAddress.create(dst));
    }

    /**
     * Test case for {@link EtherAddress#equals(Object)} and
     * {@link EtherAddress#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Long> addrSet = new HashSet<>();
        byte[] bytes = new byte[6];
        do {
            random.nextBytes(bytes);
            long addr = EtherAddress.toLong(bytes);
            addrSet.add(addr);
        } while (addrSet.size() < 100);

        HashSet<Object> set = new HashSet<>();
        for (Long l: addrSet) {
            long addr = l.longValue();
            EtherAddress ea1 = new EtherAddress(addr);
            EtherAddress ea2 = new EtherAddress(addr);
            testEquals(set, ea1, ea2);

            // Ensure that the object identity is never changed even if a
            // MAC address is cached.
            ea2.getMacAddress();
            ea2.getBytes();
            assertEquals(false, set.add(ea2));
        }

        assertEquals(addrSet.size(), set.size());
    }

    /**
     * Test case for {@link EtherAddress#toString()}.
     */
    @Test
    public void testToString() {
        byte[] bytes = new byte[6];
        for (int i = 0; i < 100; i++) {
            random.nextBytes(bytes);
            EtherAddress ea = new EtherAddress(bytes);
            String hex = ByteUtils.toHexString(bytes);
            assertEquals("EtherAddress[" + hex + "]", ea.toString());
        }
    }

    /**
     * Ensure that {@link EtherAddress} is serializable.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSerialize() throws Exception {
        byte[] bytes = new byte[6];
        for (int i = 0; i < 100; i++) {
            random.nextBytes(bytes);
            long addr = EtherAddress.toLong(bytes);
            String hex = ByteUtils.toHexString(bytes);
            MacAddress mac = new MacAddress(hex);
            EtherAddress ea = new EtherAddress(addr);
            EtherAddress ea1 = serializeTest(ea, EtherAddress.class);
            assertEquals(addr, ea1.getAddress());
            assertArrayEquals(bytes, ea1.getBytes());
            assertEquals(hex, ea1.getText());
            assertEquals(mac, ea1.getMacAddress());

            EtherAddress ea2 = serializeTest(ea1, EtherAddress.class);
            assertEquals(addr, ea2.getAddress());
            assertArrayEquals(bytes, ea2.getBytes());
            assertEquals(hex, ea2.getText());
            assertEquals(mac, ea2.getMacAddress());
        }
    }

    /**
     * Ensure that {@link EtherAddress} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        byte[] bytes = new byte[6];
        for (int i = 0; i < 100; i++) {
            random.nextBytes(bytes);
            long addr = EtherAddress.toLong(bytes);
            String hex = ByteUtils.toHexString(bytes);
            MacAddress mac = new MacAddress(hex);
            EtherAddress ea = new EtherAddress(addr);
            EtherAddress ea1 = jaxbTest(ea, EtherAddress.class, XML_ROOT);
            assertEquals(addr, ea1.getAddress());
            assertArrayEquals(bytes, ea1.getBytes());
            assertEquals(hex, ea1.getText());
            assertEquals(mac, ea1.getMacAddress());

            EtherAddress ea2 = jaxbTest(ea1, EtherAddress.class, XML_ROOT);
            assertEquals(addr, ea2.getAddress());
            assertArrayEquals(bytes, ea2.getBytes());
            assertEquals(hex, ea2.getText());
            assertEquals(mac, ea2.getMacAddress());
        }

        // Ensure that leading and trailing whitespaces are eliminated.
        long addr = 0xabcdef123456L;
        String hex = "ab:cd:ef:12:34:56";
        StringBuilder builder = new StringBuilder(XML_DECLARATION);
        builder.append('<').append(XML_ROOT).append('>').
            append("\n    \t  ").append(hex.toUpperCase(Locale.ENGLISH)).
            append("     \t\t\t\t      \r\n").
            append("</").append(XML_ROOT).append('>');
        String xml = builder.toString();

        ByteArrayInputStream in = new ByteArrayInputStream(xml.getBytes());
        Unmarshaller um = createUnmarshaller(EtherAddress.class);
        Object o = um.unmarshal(in);
        assertTrue(o instanceof EtherAddress);
        EtherAddress ea = (EtherAddress)o;
        assertEquals(addr, ea.getAddress());
        assertEquals(hex, ea.getText());

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, EtherAddress.class,
                      new XmlValueType(XML_ROOT, EtherAddress.class));
    }
}
