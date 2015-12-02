/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Random;
import java.util.TreeSet;

import javax.xml.bind.Marshaller;
import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.ByteUtils;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.XmlDataType;
import org.opendaylight.vtn.manager.internal.XmlValueType;
import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.input.DataFlowSource;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.get.data.flow.input.DataFlowSourceBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.SourceHostFlowsKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHostDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.vlan.host.desc.set.VlanHostDescListBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;
import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * JUnit test for {@link MacVlan}.
 */
public class MacVlanTest extends TestBase {
    /**
     * Root XML element name associated with {@link MacVlan} class.
     */
    private static final String  XML_ROOT = "mac-vlan";

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
     * Return a list of {@link XmlDataType} instances that specifies XML node
     * types mapped to a {@link MacVlan} instance.
     *
     * @param name    The name of the target node.
     * @param parent  Path to the parent node.
     * @return  A list of {@link XmlDataType} instances.
     */
    public static List<XmlDataType> getXmlDataTypes(String name,
                                                    String ... parent) {
        List<XmlDataType> dlist = new ArrayList<>();
        Collections.addAll(
            dlist,
            new XmlValueType("address",
                             EtherAddress.class).add(name).prepend(parent),
            new XmlValueType("vlan-id",
                             Integer.class).add(name).prepend(parent));

        return dlist;
    }

    /**
     * Test case for getter methods.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testGetter() throws Exception {
        int[] vlans = {0, 1, 1000, 4095};
        for (EtherAddress eaddr: createEtherAddresses()) {
            for (int vlan: vlans) {
                byte[] mac;
                long macLongVal;
                if (eaddr == null) {
                    mac = null;
                    macLongVal = 0;
                } else {
                    mac = eaddr.getBytes();
                    macLongVal = eaddr.getAddress();
                }
                MacVlan mv = new MacVlan(mac, vlan);
                long encoded = ((macLongVal & MASK_MAC) << NBITS_VLAN_ID) |
                    ((long)vlan & MASK_VLAN_ID);
                VlanHostDesc vhdesc;
                if (eaddr == null) {
                    vhdesc = new VlanHostDesc("@" + vlan);
                } else {
                    vhdesc = new VlanHostDesc(eaddr.getText() + "@" + vlan);
                }

                VlanHostDescList vhdlist = new VlanHostDescListBuilder().
                    setHost(vhdesc).build();

                assertEquals(macLongVal, mv.getAddress());
                assertEquals(vlan, mv.getVlanId());
                assertEquals(encoded, mv.getEncodedValue());
                assertEquals(vhdesc, mv.getVlanHostDesc());
                assertEquals(vhdlist, mv.getVlanHostDescList());
                assertEquals(eaddr, mv.getEtherAddress());

                mv = new MacVlan(macLongVal | ~MASK_MAC, vlan);
                assertEquals(macLongVal, mv.getAddress());
                assertEquals(vlan, mv.getVlanId());
                assertEquals(encoded, mv.getEncodedValue());
                assertEquals(vhdesc, mv.getVlanHostDesc());
                assertEquals(vhdlist, mv.getVlanHostDescList());
                assertEquals(eaddr, mv.getEtherAddress());

                mv = new MacVlan(encoded | ~MASK_ENCODED);
                assertEquals(macLongVal, mv.getAddress());
                assertEquals(vlan, mv.getVlanId());
                assertEquals(encoded, mv.getEncodedValue());
                assertEquals(vhdesc, mv.getVlanHostDesc());
                assertEquals(vhdlist, mv.getVlanHostDescList());
                assertEquals(eaddr, mv.getEtherAddress());

                MacAddress maddr = (eaddr == null)
                    ? null : eaddr.getMacAddress();
                MacVlan mv1 = new MacVlan(maddr, (int)vlan);
                assertEquals(macLongVal, mv1.getAddress());
                assertEquals(vlan, mv1.getVlanId());
                assertEquals(encoded, mv1.getEncodedValue());
                assertEquals(vhdesc, mv1.getVlanHostDesc());
                assertEquals(vhdlist, mv1.getVlanHostDescList());
                assertEquals(eaddr, mv1.getEtherAddress());
                assertEquals(maddr, mv1.getMacAddress());
                assertEquals(mv, mv1);

                DataFlowSource src = mv.getDataFlowSource();
                VlanId vid = new VlanId((int)vlan);
                assertEquals(maddr, src.getMacAddress());
                assertEquals(vid, src.getVlanId());

                mv1 = new MacVlan(src);
                assertEquals(macLongVal, mv1.getAddress());
                assertEquals(vlan, mv1.getVlanId());
                assertEquals(encoded, mv1.getEncodedValue());
                assertEquals(vhdesc, mv1.getVlanHostDesc());
                assertEquals(vhdlist, mv1.getVlanHostDescList());
                assertEquals(eaddr, mv1.getEtherAddress());
                assertEquals(maddr, mv1.getMacAddress());
                assertEquals(mv, mv1);

                SourceHostFlowsKey skey = mv.getSourceHostFlowsKey();
                if (maddr == null) {
                    assertEquals(null, skey);
                } else {
                    assertEquals(maddr, skey.getMacAddress());
                    assertEquals(vid, skey.getVlanId());
                }
            }
        }

        // Only lower 48-bits in a long value is used as MAC address, and
        // only lower 12-bits in a int value is used as VLAN ID.
        long mac = -1L;
        int vlan = -1;
        long encoded = ((mac & MASK_MAC) << NBITS_VLAN_ID) |
            ((long)vlan & MASK_VLAN_ID);
        MacVlan mvlan = new MacVlan(mac, vlan);
        assertEquals(MASK_MAC, mvlan.getAddress());
        assertEquals(MASK_VLAN_ID, mvlan.getVlanId());
        assertEquals(MASK_ENCODED, mvlan.getEncodedValue());

        mvlan = new MacVlan(-1L);
        assertEquals(MASK_MAC, mvlan.getAddress());
        assertEquals(MASK_VLAN_ID, mvlan.getVlanId());
        assertEquals(MASK_ENCODED, mvlan.getEncodedValue());

        try {
            new MacVlan((DataFlowSource)null);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals("vlan-host cannot be null", e.getMessage());
        }

        MacAddress maddr = new MacAddress("00:11:22:33:44:55");
        DataFlowSource src = new DataFlowSourceBuilder().
            setMacAddress(maddr).build();
        try {
            new MacVlan(src);
            unexpected();
        } catch (RpcException e) {
            assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
            assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
            assertEquals("VLAN ID cannot be null", e.getMessage());
        }
    }

    /**
     * Test case for {@link MacVlan#MacVlan(VlanHostDesc)} and
     * {@link MacVlan#MacVlan(String)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testConstructVlanHostDesc() throws Exception {
        int[] vlans = {0, 1, 10, 1000, 4095};

        List<EtherAddress> etherAddrs = createEtherAddresses();
        for (EtherAddress eaddr: etherAddrs) {
            for (int vlan: vlans) {
                MacAddress maddr = null;
                String desc = "@" + vlan;
                if (eaddr != null) {
                    maddr = eaddr.getMacAddress();
                    desc = maddr.getValue() + desc;
                }

                VlanHostDesc vhd = new VlanHostDesc(desc);
                MacVlan mv = new MacVlan(vhd);
                assertEquals(maddr, mv.getMacAddress());
                assertEquals(vlan, mv.getVlanId());
                assertEquals(desc, mv.toString());

                // Lower-cased string should be cached.
                String upper = desc.toUpperCase(Locale.ENGLISH);
                mv = new MacVlan(upper);
                assertEquals(maddr, mv.getMacAddress());
                assertEquals(vlan, mv.getVlanId());
                assertEquals(desc, mv.toString());
            }
        }

        // Null argument.
        RpcErrorTag etag = RpcErrorTag.MISSING_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        String emsg = "vlan-host-desc cannot be null";
        VlanHostDesc vhdesc = null;
        try {
            new MacVlan(vhdesc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Invalid VLAN ID.
        etag = RpcErrorTag.BAD_ELEMENT;
        long[] badVids = {
            4096, 4097, 10000, 30000, 12345678, Integer.MAX_VALUE,
            0x80000000L, 0x1000000000L, Long.MAX_VALUE,
        };
        for (long vid: badVids) {
            vhdesc = new VlanHostDesc("00:aa:bb:cc:dd:ee@" + vid);
            emsg = "Invalid VLAN ID in vlan-host-desc: " + vhdesc.getValue();
            try {
                new MacVlan(vhdesc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(emsg, e.getMessage());

                Throwable cause = e.getCause();
                if (vid <= Integer.MAX_VALUE) {
                    assertTrue(cause instanceof RpcException);
                    RpcException re = (RpcException)cause;
                    assertEquals(etag, re.getErrorTag());
                    assertEquals(vtag, re.getVtnErrorTag());
                    assertEquals("Invalid VLAN ID: " + vid, re.getMessage());
                } else {
                    assertTrue(cause instanceof NumberFormatException);
                }
            }
        }

        // Broadcast address.
        vhdesc = new VlanHostDesc("ff:ff:ff:ff:ff:ff@0");
        emsg = "Broadcast address cannot be specified: ff:ff:ff:ff:ff:ff";
        try {
            new MacVlan(vhdesc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Zero MAC address.
        vhdesc = new VlanHostDesc("00:00:00:00:00:00@0");
        emsg = "Zero MAC address cannot be specified: 00:00:00:00:00:00";
        try {
            new MacVlan(vhdesc);
            unexpected();
        } catch (RpcException e) {
            assertEquals(etag, e.getErrorTag());
            assertEquals(vtag, e.getVtnErrorTag());
            assertEquals(emsg, e.getMessage());
        }

        // Multicast address.
        for (EtherAddress eaddr: etherAddrs) {
            if (eaddr == null) {
                continue;
            }

            for (int vlan: vlans) {
                long mac = eaddr.getAddress() | BIT_MULTICAST;
                EtherAddress eaddr1 = new EtherAddress(mac);
                String text = eaddr1.getText();
                emsg = "Multicast address cannot be specified: " + text;
                vhdesc = new VlanHostDesc(text + "@" + vlan);
                try {
                    new MacVlan(vhdesc);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(etag, e.getErrorTag());
                    assertEquals(vtag, e.getVtnErrorTag());
                    assertEquals(emsg, e.getMessage());
                }
            }
        }
    }

    /**
     * Test case for {@link MacVlan#getEtherAddress()} and
     * {@link MacVlan#setEtherAddress(EtherAddress)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetEtherAddress() throws Exception {
        long addr = EtherAddress.BROADCAST.getAddress();
        int vlan = 4095;
        MacVlan mv = new MacVlan(addr, vlan);
        assertEquals(EtherAddress.BROADCAST, mv.getEtherAddress());
        assertEquals(vlan, mv.getVlanId());

        long[] macAddrs = {
            1L, 2L, 0x001122334455L, 0xa0b0c0d0e0f0L, 0x012345abcdefL,
        };
        for (long mac: macAddrs) {
            EtherAddress eaddr = new EtherAddress(mac);
            mv.setEtherAddress(eaddr);
            assertEquals(eaddr, mv.getEtherAddress());
            assertEquals(vlan, mv.getVlanId());
            mv.verify();
        }

        mv.setEtherAddress(null);
        assertEquals(null, mv.getEtherAddress());
        assertEquals(MacVlan.UNDEFINED, mv.getAddress());
        assertEquals(vlan, mv.getVlanId());
        mv.verify();
    }

    /**
     * Test case for {@link MacVlan#getVlanId()} and
     * {@link MacVlan#setVlanId(int)}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetVlan() throws Exception {
        EtherAddress eaddr = EtherAddress.BROADCAST;
        int vlan = 4095;
        MacVlan mv = new MacVlan(eaddr.getAddress(), vlan);
        assertEquals(eaddr, mv.getEtherAddress());
        assertEquals(vlan, mv.getVlanId());

        int[] vlanIds = {
            0, 1, 3, 5, 100, 444, 890, 1014, 3000, 4094, vlan,
        };
        for (int vid: vlanIds) {
            mv.setVlanId(vid);
            assertEquals(eaddr, mv.getEtherAddress());
            assertEquals(vid, mv.getVlanId());
            mv.verify();
        }

        // Invalid VLAN IDs.
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        int[] invalid = {
            Integer.MIN_VALUE, -12345678, -30000, -100, -3, -2, -1,
            4096, 4097, 5000, 8888, 10000, 3333333, Integer.MAX_VALUE,
        };
        for (int vid: invalid) {
            mv.setVlanId(vid);
            assertEquals(eaddr, mv.getEtherAddress());
            assertEquals(vlan, mv.getVlanId());

            String msg = "Invalid mac-vlan: Invalid VLAN ID: " + vid;
            try {
                mv.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
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
        int[] vlans = {0, 1, 10, 1000, 4095};
        List<EtherAddress> ethers = createEtherAddresses();
        for (EtherAddress eaddr: ethers) {
            for (int vlan: vlans) {
                byte[] mac = (eaddr == null) ? null : eaddr.getBytes();
                MacVlan mv1 = new MacVlan(mac, vlan);
                MacVlan mv2 = new MacVlan(mac, vlan);
                testEquals(set, mv1, mv2);
            }
        }

        int required = ethers.size() * vlans.length;
        assertEquals(required, set.size());

        set.clear();
        for (EtherAddress eaddr: ethers) {
            for (int vlan: vlans) {
                byte[] mac;
                long macLongVal;
                if (eaddr == null) {
                    mac = null;
                    macLongVal = 0;
                } else {
                    mac = eaddr.getBytes();
                    macLongVal = eaddr.getAddress();
                }
                MacVlan mv1 = new MacVlan(macLongVal, vlan);
                MacVlan mv2 = new MacVlan(macLongVal, vlan);
                testEquals(set, mv1, mv2);
            }
        }

        assertEquals(required, set.size());
    }

    /**
     * Test method for {@link MacVlan#toString()}.
     */
    @Test
    public void testToString() {
        int[] vlans = {0, 1, 10, 1000, 4095};

        for (EtherAddress eaddr: createEtherAddresses()) {
            for (int vlan: vlans) {
                byte[] mac;
                long lmac;
                if (eaddr == null) {
                    mac = null;
                    lmac = 0;
                } else {
                    mac = eaddr.getBytes();
                    lmac = eaddr.getAddress();
                }
                MacVlan mvlan = new MacVlan(mac, vlan);
                String c = (lmac == MacVlan.UNDEFINED)
                    ? "" : ByteUtils.toHexString(mac);

                String expected = c + "@" + vlan;
                String value = mvlan.toString();
                assertEquals(expected, value);

                // Result should be cached.
                assertSame(value, mvlan.toString());
            }
        }
    }

    /**
     * Test case for {@link MacVlan#compareTo(MacVlan)}.
     */
    @Test
    public void testCompareTo() {
        HashSet<MacVlan> hset = new HashSet<>();

        // Create test data.
        MacVlan least = new MacVlan(0L);
        MacVlan greatest = new MacVlan(-1L);
        hset.add(least);
        hset.add(greatest);
        do {
            long mac = rand.nextLong();
            int vlan = rand.nextInt(4);
            MacVlan mvlan = new MacVlan(mac, vlan);
            hset.add(mvlan);
        } while (hset.size() < 50);

        // Sort instances using TreeSet.
        TreeSet<MacVlan> tset = new TreeSet<>(hset);
        assertEquals(hset.size(), tset.size());

        // Ensure that MacVlan instances are sorted as expected.
        long prevEncoded = -1;
        long prevMac = -1;
        int prevVlan = -1;
        MacVlan last = null;
        for (MacVlan mvlan: tset) {
            long encoded = mvlan.getEncodedValue();
            long mac = mvlan.getAddress();
            int vlan = mvlan.getVlanId();
            last = mvlan;

            if (prevEncoded == -1) {
                assertSame(least, mvlan);
                assertEquals(0L, encoded);
                assertEquals(0L, mac);
                assertEquals(0, vlan);
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
        assertEquals(MASK_MAC, last.getAddress());
        assertEquals(MASK_VLAN_ID, last.getVlanId());
    }

    /**
     * Test case for {@link MacVlan#checkMacMap()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testCheckMacMap() throws Exception {
        long[] addrs = {
            MacVlan.UNDEFINED,
            0x1L,
            0x001122334455L,
            0xa0b0c0d0e0f0L,
        };
        int[] vlanIds = {
            0, 1, 3, 5, 100, 444, 890, 1014, 3000, 4094, 4095,
        };

        for (long mac: addrs) {
            for (int vid: vlanIds) {
                MacVlan mv = new MacVlan(mac, vid);
                mv.checkMacMap();
            }
        }

        Map<MacVlan, String> badCases = new HashMap<>();
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;

        // Broadcast address.
        MacVlan mv = new MacVlan(0xffffffffffffL, 0);
        String msg = "Broadcast address cannot be specified: ff:ff:ff:ff:ff:ff";
        assertEquals(null, badCases.put(mv, msg));

        // Multicast address.
        long[] maddrs = {
            0x010000000000L,
            0xfffffffffffeL,
            0x1b012345abc0L,
        };
        for (long mac: maddrs) {
            EtherAddress eaddr = new EtherAddress(mac);
            mv = new MacVlan(mac, 0);
            msg = "Multicast address cannot be specified: " +
                eaddr.getText();
            assertEquals(null, badCases.put(mv, msg));
        }

        for (Map.Entry<MacVlan, String> entry: badCases.entrySet()) {
            mv = entry.getKey();
            try {
                mv.checkMacMap();
                System.err.printf("*** mv=%s, msg=%s\n", mv, entry.getValue());
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(entry.getValue(), e.getMessage());
            }
        }

        // Invalid VLAN IDs.
        Class<MacVlan> type = MacVlan.class;
        Unmarshaller um = createUnmarshaller(type);
        int[] invalid = {
            Integer.MIN_VALUE, -3333333, -1234, -2, -1,
            4096, 4097, 9999, 123456789, Integer.MAX_VALUE,
        };
        for (int vid: invalid) {
            String xml = new XmlNode(XML_ROOT).
                add(new XmlNode("address", "00:11:22:33:44:55")).
                add(new XmlNode("vlan-id", vid)).
                toString();
            mv = unmarshal(um, xml, type);
            msg = "Invalid mac-vlan: Invalid VLAN ID: " + vid;
            try {
                mv.checkMacMap();
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }
    }

    /**
     * Ensure that {@link MacVlan} is mapped to XML root element.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testJAXB() throws Exception {
        EtherAddress[] addrs = {
            null,
            new EtherAddress(0x1L),
            new EtherAddress(0x001122334455L),
            new EtherAddress(0xa0b0c0d0e0f0L),
            new EtherAddress(0xffffffffffffL),
        };
        int[] vlanIds = {
            0, 1, 3, 5, 100, 444, 890, 1014, 3000, 4094, 4095,
        };

        Class<MacVlan> type = MacVlan.class;
        Marshaller m = createMarshaller(type);
        Unmarshaller um = createUnmarshaller(type);

        for (EtherAddress eaddr: addrs) {
            for (int vid: vlanIds) {
                long mac = (eaddr == null)
                    ? MacVlan.UNDEFINED : eaddr.getAddress();
                MacVlan mv = new MacVlan(mac, vid);
                MacVlan mv1 = jaxbTest(mv, type, m, um, XML_ROOT);
                assertEquals(eaddr, mv1.getEtherAddress());
                assertEquals(vid, mv1.getVlanId());
            }
        }

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(um, type, getXmlDataTypes(XML_ROOT));

        // Invalid VLAN IDs.
        RpcErrorTag etag = RpcErrorTag.BAD_ELEMENT;
        VtnErrorTag vtag = VtnErrorTag.BADREQUEST;
        int[] invalid = {
            Integer.MIN_VALUE, -3000000, -1234, -100, -3, -2, -1,
            4096, 4097, 5000, 8888, 1000000, Integer.MAX_VALUE,
        };
        for (int vid: invalid) {
            String xml = new XmlNode(XML_ROOT).
                add(new XmlNode("address", "00:11:22:33:44:55")).
                add(new XmlNode("vlan-id", vid)).
                toString();

            MacVlan mv = unmarshal(um, xml, type);
            String msg = "Invalid mac-vlan: Invalid VLAN ID: " + vid;
            try {
                mv.verify();
                unexpected();
            } catch (RpcException e) {
                assertEquals(etag, e.getErrorTag());
                assertEquals(vtag, e.getVtnErrorTag());
                assertEquals(msg, e.getMessage());
            }
        }
    }
}
