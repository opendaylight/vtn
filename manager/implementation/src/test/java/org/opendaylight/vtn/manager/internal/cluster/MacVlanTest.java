/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.HashSet;
import java.util.List;
import java.util.ArrayList;
import java.util.Random;
import java.util.TreeSet;

import org.junit.Test;

import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.TestDataLink;
import org.opendaylight.vtn.manager.internal.TestDataLinkHost;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * JUnit test for {@link MacVlan}.
 */
public class MacVlanTest extends TestBase {
    /**
     * Mask value which represents valid VLAN ID bits in a long integer.
     */
    private static final long  MASK_VLAN_ID = 0xfffL;

    /**
     * Mask value which represents valid MAC address bits in a long integer.
     */
    private static final long  MASK_MAC = 0x0000ffffffffffffL;

    /**
     * Mask value which represents valid bits in a long value encoded from
     * {@link MacVlan} instance.
     */
    private static final long  MASK_ENCODED = 0x0fffffffffffffffL;

    /**
     * A multicast bit in a MAC address.
     */
    private static final long BIT_MULTICAST = 0x0000010000000000L;

    /**
     * The number of bits in a valid VLAN ID.
     */
    private static final int  NBITS_VLAN_ID = 12;

    /**
     * A pseudo random number generator.
     */
    private final Random  rand = new Random();

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        assertEquals(MASK_VLAN_ID, MacVlan.MASK_VLAN_ID);

        short[] vlans = new short[] {0, 1, 1000, 4095};
        for (EthernetAddress ea : createEthernetAddresses()) {
            for (short vlan : vlans) {
                byte[] mac = (ea == null) ? null : ea.getValue();
                MacVlan mv = new MacVlan(mac, vlan);
                long macLongVal = NetUtils.byteArray6ToLong(mac);
                long encoded = ((macLongVal & MASK_MAC) << NBITS_VLAN_ID) |
                    ((long)vlan & MASK_VLAN_ID);
                assertEquals(macLongVal, mv.getMacAddress());
                assertEquals(vlan, mv.getVlan());
                assertEquals(encoded, mv.getEncodedValue());

                mv = new MacVlan(macLongVal | ~MASK_MAC, vlan);
                assertEquals(macLongVal, mv.getMacAddress());
                assertEquals(vlan, mv.getVlan());
                assertEquals(encoded, mv.getEncodedValue());

                mv = new MacVlan(encoded | ~MASK_ENCODED);
                assertEquals(macLongVal, mv.getMacAddress());
                assertEquals(vlan, mv.getVlan());
                assertEquals(encoded, mv.getEncodedValue());

                try {
                    EthernetHost ehost = new EthernetHost(ea, vlan);
                    mv = new MacVlan(ehost);
                    assertEquals(macLongVal, mv.getMacAddress());
                    assertEquals(vlan, mv.getVlan());
                    assertEquals(encoded, mv.getEncodedValue());
                    assertEquals(ehost, mv.getEthernetHost());
                } catch (Exception e) {
                    unexpected(e);
                }
            }
        }

        // Only lower 48-bits in a long value is used as MAC address, and
        // only lower 12-bits in a short value is used as VLAN ID.
        long mac = -1L;
        short vlan = (short)-1;
        long encoded = ((mac & MASK_MAC) << NBITS_VLAN_ID) |
            ((long)vlan & MASK_VLAN_ID);
        MacVlan mvlan = new MacVlan(mac, vlan);
        assertEquals(MASK_MAC, mvlan.getMacAddress());
        assertEquals(MASK_VLAN_ID, mvlan.getVlan());
        assertEquals(MASK_ENCODED, mvlan.getEncodedValue());

        mvlan = new MacVlan(-1L);
        assertEquals(MASK_MAC, mvlan.getMacAddress());
        assertEquals(MASK_VLAN_ID, mvlan.getVlan());
        assertEquals(MASK_ENCODED, mvlan.getEncodedValue());

        // Specify invalid DataLinkHost.
        try {
            mvlan = new MacVlan((EthernetHost)null);
            fail("An exception must be thrown");
        } catch (VTNException e) {
            Status status = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, status.getCode());
        }

        TestDataLink dladdr = new TestDataLink("addr");
        TestDataLinkHost dlhost = new TestDataLinkHost(dladdr);
        try {
            mvlan = new MacVlan(dlhost);
            fail("An exception must be thrown");
        } catch (VTNException e) {
            Status status = e.getStatus();
            assertEquals(StatusCode.BADREQUEST, status.getCode());
        }

        ArrayList<Long> invaddrs = new ArrayList<Long>();
        invaddrs.add(0L);
        invaddrs.add(MASK_MAC);
        for (int i = 0; i < 10; i++) {
            invaddrs.add((rand.nextLong() & MASK_MAC) | BIT_MULTICAST);
        }
        for (Long addr: invaddrs) {
            byte[] raw = NetUtils.longToByteArray6(addr.longValue());
            EthernetAddress eth = null;
            try {
                eth = new EthernetAddress(raw);
            } catch (Exception e) {
                unexpected(e);
            }

            EthernetHost ehost = new EthernetHost(eth, (short)0);
            try {
                mvlan = new MacVlan(dlhost);
                fail("An exception must be thrown");
            } catch (VTNException e) {
                Status status = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, status.getCode());
            }
        }

        for (short v = -10; v < 0; v++) {
            EthernetHost ehost = new EthernetHost(null, v);
            try {
                mvlan = new MacVlan(dlhost);
                fail("An exception must be thrown");
            } catch (VTNException e) {
                Status status = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, status.getCode());
            }
        }
        for (short v = 4096; v < 4100; v++) {
            EthernetHost ehost = new EthernetHost(null, v);
            try {
                mvlan = new MacVlan(dlhost);
                fail("An exception must be thrown");
            } catch (VTNException e) {
                Status status = e.getStatus();
                assertEquals(StatusCode.BADREQUEST, status.getCode());
            }
        }
    }

    /**
     * Test method for
     * {@link MacVlan#equals(Object)} and
     * {@link MacVlan#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();
        short[] vlans = {0, 1, 10, 1000, 4095};
        List<EthernetAddress> ethers = createEthernetAddresses();

        for (EthernetAddress ea : ethers) {
            for (short vlan: vlans) {
                byte[] mac = (ea == null) ? null : ea.getValue();
                MacVlan mv1 = new MacVlan(mac, vlan);
                MacVlan mv2 = new MacVlan(mac, vlan);
                testEquals(set, mv1, mv2);
            }
        }

        int required = ethers.size() * vlans.length;
        assertEquals(required, set.size());

        set.clear();
        for (EthernetAddress ea : ethers) {
            for (short vlan: vlans) {
                byte[] mac = (ea == null) ? null : ea.getValue();
                long macLongVal = NetUtils.byteArray6ToLong(mac);
                MacVlan mv1 = new MacVlan(macLongVal, vlan);
                MacVlan mv2 = new MacVlan(macLongVal, vlan);
                testEquals(set, mv1, mv2);
            }
        }

        assertEquals(required, set.size());
    }

    /**
     * Test method for {@link MacVlan#toString()} and
     * {@link MacVlan#appendContents(StringBuilder)}.
     */
    @Test
    public void testToString() {
        String prefix = "MacVlan[";
        String suffix = "]";
        short[] vlans = {0, 1, 10, 1000, 4095};

        for (EthernetAddress ea : createEthernetAddresses()) {
            for (short vlan: vlans) {
                byte[] mac = (ea == null) ? null : ea.getValue();
                MacVlan mvlan = new MacVlan(mac, vlan);
                long lmac = NetUtils.byteArray6ToLong(mac);
                String c = (lmac == MacVlan.UNDEFINED)
                    ? null : "addr=" + HexEncode.bytesToHexStringFormat(mac);
                String v = "vlan=" + vlan;

                StringBuilder builder = new StringBuilder();
                mvlan.appendContents(builder);
                String required = joinStrings(null, null, ",", c, v);
                assertEquals(required, builder.toString());

                required = joinStrings(prefix, suffix, ",", c, v);
                assertEquals(required, mvlan.toString());
            }
        }
    }

    /**
     * Test case for {@link MacVlan#compareTo(MacVlan)}.
     */
    @Test
    public void testCompareTo() {
        HashSet hset = new HashSet<MacVlan>();

        // Create test data.
        MacVlan least = new MacVlan(0L);
        MacVlan greatest = new MacVlan(-1L);
        hset.add(least);
        hset.add(greatest);
        do {
            long mac = rand.nextLong();
            short vlan = (short)rand.nextInt(4);
            MacVlan mvlan = new MacVlan(mac, vlan);
            hset.add(mvlan);
        } while (hset.size() < 50);

        // Sort instances using TreeSet.
        TreeSet<MacVlan> tset = new TreeSet<MacVlan>(hset);
        assertEquals(hset.size(), tset.size());

        // Ensure that MacVlan instances are sorted as expected.
        long prevEncoded = -1;
        long prevMac = -1;
        short prevVlan = (short)-1;
        MacVlan last = null;
        for (MacVlan mvlan: tset) {
            long encoded = mvlan.getEncodedValue();
            long mac = mvlan.getMacAddress();
            short vlan = mvlan.getVlan();
            last = mvlan;

            if (prevEncoded == (short)-1) {
                assertSame(least, mvlan);
                assertEquals(0L, encoded);
                assertEquals(0L, mac);
                assertEquals((short)0, vlan);
            } else {
                assertTrue(encoded > prevEncoded);
                if (mac == prevMac) {
                    assertTrue(vlan > prevVlan);
                } else {
                    assertTrue(mac > prevMac);
                }
            }

            prevEncoded = encoded;
            prevVlan = vlan;
            prevMac = mac;
        }

        assertSame(greatest, last);
        assertEquals(MASK_ENCODED, last.getEncodedValue());
        assertEquals(MASK_MAC, last.getMacAddress());
        assertEquals(MASK_VLAN_ID, last.getVlan());
    }

    /**
     * Ensure that {@link MacVlan} is serializable.
     */
    @Test
    public void testSerialize() {
        short[] vlans = {0, 1, 100, 1000, 4095};
        for (EthernetAddress ea : createEthernetAddresses()) {
            for (short vlan: vlans) {
                byte[] mac = (ea == null) ? null : ea.getValue();
                MacVlan mvlan = new MacVlan(mac, vlan);
                serializeTest(mvlan);
            }
        }
    }
}
