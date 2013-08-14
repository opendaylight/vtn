/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;

import org.apache.felix.dm.impl.ComponentImpl;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.VTNException;

/**
 * JUnit test for {@link MacAddressTable}
 */
public class MacAddressTableTest extends TestBase {
    private static VTNManagerImpl vtnMgr = null;

    /**
     * setup a test environment for MacAddressTableTest
     */
    @BeforeClass
    public static void beforeClass() {
        vtnMgr = new VTNManagerImpl();
        ComponentImpl c = new ComponentImpl(null, null, null);
        GlobalResourceManager grsc = new GlobalResourceManager();

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        grsc.init(c);
        vtnMgr.init(c);
        vtnMgr.setResourceManager(grsc);
    }

    /**
     * cleanup a test environment
     */
    @AfterClass
    public static void afterClass() {
        // nothing to do.
    }

    /**
     * Test case for {@link MacAddressTable#getTableKey(byte [])}
     */
    @Test
    public void testGetTableKey() {
        for (EthernetAddress ea: createEthernetAddresses(false)) {
            byte [] value = ea.getValue();
            Long lvalue = MacAddressTable.getTableKey(ea.getValue());

            int len = value.length;
            for (int i = 0; i < len; i++) {
                assertTrue(value[len - i - 1] == (byte)((lvalue.longValue() >> (i * 8)) & 0xff));
            }
            assertTrue(lvalue.longValue() < (long)(1L << (len * 8)));
        }
    }

    /**
     * Test case for {@link MacAddressTable#MacAddressTable(VTNManagerImpl, String, int)} and
     * {@link MacAddressTable#destroy()}
     */
    @Test
    public void testMacAddressTable() {
        VTNManagerImpl mgr = vtnMgr;

        MacAddressTable tbl = new MacAddressTable(mgr, "Table", 600);
        assertNotNull(tbl);
        tbl.destroy(mgr);

        tbl = new MacAddressTable(mgr, "Table", 10);
        assertNotNull(tbl);
        tbl.destroy(mgr);

        tbl = new MacAddressTable(mgr, "Table", 1000000);
        assertNotNull(tbl);
        tbl.destroy(mgr);
    }

    /**
     * Test case for {@link MacAddressTable#setAgeInterval(VTNManagerImpl, int)}
     */
    @Test
    public void testSetAgeInterval() {
        VTNManagerImpl mgr = vtnMgr;

        MacAddressTable tbl = new MacAddressTable(mgr, "Table", 600);
        tbl.setAgeInterval(mgr, 10);
        tbl.setAgeInterval(mgr, 1000000);
        tbl.setAgeInterval(mgr, 1000000);
        tbl.destroy(mgr);
    }

    /**
     * Test case for
     * {@link MacAddressTable#add(VTNManagerImpl, PacketContext)},
     * {@link MacAddressTable#get(PacketContext)},
     * {@link MacAddressTable#remove(PacketContext)},
     * {@link MacAddressTable#getEntry(DataLinkAddress)},
     * {@link MacAddressTable#getEntries()}
     */
    @Test
    public void testAddGetRemoveEntry() {
        VTNManagerImpl mgr = vtnMgr;
        MacAddressTable tbl = new MacAddressTable(mgr, "Table", 600);
        MacAddressTable tall = new MacAddressTable(mgr, "Table", 600);
        MacTableEntry tent = null;
        MacAddressEntry mae = null;

        byte iphost = 1;
        short vlan = 4095;
        List<NodeConnector> connectors = createNodeConnectors(3, false);

        for (EthernetAddress ea: createEthernetAddresses(false)) {
            byte [] bytes = ea.getValue();
            byte [] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                        bytes[3], bytes[4], bytes[5]};
            byte [] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                        (byte)0xff, (byte)0xff, (byte)0xff};
            byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)iphost};
            byte [] sender2 = new byte[] {(byte)192, (byte)168, (byte)0, (byte)(iphost + 100)};
            byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

            // default
            PacketContext dpctx = createARPPacketContext(src, dst, sender, target,
                                          (short)-1, connectors.get(0), ARP.REQUEST);
            // replaced src and dst
            PacketContext rpctx = createARPPacketContext(dst, src, target, sender,
                                          (short)-1, connectors.get(0), ARP.REQUEST);
            // different sender address
            PacketContext ippctx = createARPPacketContext(src, dst, sender2, target,
                                          (short)-1, connectors.get(0), ARP.REQUEST);
            // have vlan tag
            PacketContext vlanpctx = createARPPacketContext(src, dst, sender, target,
                                          vlan, connectors.get(0), ARP.REQUEST);
            // different node connector
            PacketContext ncpctx = createARPPacketContext(src, dst, sender, target,
                                          (short)-1, connectors.get(1), ARP.REQUEST);

            // get from empty table.
            tent = tbl.get(dpctx);
            assertNull(tent);
            try {
                mae = tbl.getEntry(ea);
            } catch (VTNException e) {
                unexpected(e);
            }
            assertNull(mae);

            // add() and get() one data
            tbl.add(mgr, dpctx);
            tent = tbl.get(dpctx);
            assertNull(tent);
            tent = tbl.get(rpctx);
            assertNotNull(tent);
            assertTrue(tent.clearUsed());
            assertEquals(connectors.get(0), tent.getPort());
            assertEquals(0, tent.getVlan());

            // getEntry()
            try {
                mae = tbl.getEntry(ea);
            } catch (VTNException e) {
                unexpected(e);
            }
            assertNotNull(mae);
            assertEquals(ea, mae.getAddress());
            assertEquals(0, mae.getVlan());
            assertEquals(connectors.get(0), mae.getNodeConnector());
            Set<InetAddress> iplist = mae.getInetAddresses();
            assertTrue(iplist.size() == 1);
            assertArrayEquals(sender, iplist.iterator().next().getAddress());

            // add same packet
            assertFalse(tent.clearUsed());
            tbl.add(mgr, dpctx);
            assertTrue(tent.clearUsed());

            // add packet IP changed
            tbl.add(mgr, ippctx);
            tent = tbl.get(rpctx);
            assertNotNull(tent);
            assertEquals(connectors.get(0), tent.getPort());
            assertEquals(0, tent.getVlan());
            try {
                mae = tbl.getEntry(ea);
            } catch (VTNException e) {
                unexpected(e);
            }
            iplist = mae.getInetAddresses();
            assertTrue(iplist.size() == 2);
            InetAddress ip = null, ip2 = null;
            try {
                ip = InetAddress.getByAddress(sender);
                ip2 = InetAddress.getByAddress(sender2);
            } catch (UnknownHostException e) {
                unexpected(e);
            }
            assertTrue(iplist.contains(ip));
            assertTrue(iplist.contains(ip2));
            assertFalse(ip.equals(ip2));

            tbl.add(mgr, dpctx);

            // add packet vlan changed
            tbl.add(mgr, vlanpctx);
            tent = tbl.get(rpctx);
            assertNotNull(tent);
            assertEquals(connectors.get(0), tent.getPort());
            assertEquals(vlan, tent.getVlan());
            try {
                mae = tbl.getEntry(ea);
            } catch (VTNException e) {
                unexpected(e);
            }
            iplist = mae.getInetAddresses();
            assertTrue(iplist.size() == 1);
            assertArrayEquals(sender, iplist.iterator().next().getAddress());

            tbl.add(mgr, dpctx);

            // add packet setted different nodeconnector
            tbl.add(mgr, ncpctx);
            tent = tbl.get(rpctx);
            assertNotNull(tent);
            assertEquals(connectors.get(1), tent.getPort());
            assertEquals(0, tent.getVlan());
            try {
                mae = tbl.getEntry(ea);
            } catch (VTNException e) {
                unexpected(e);
            }
            iplist = mae.getInetAddresses();
            assertTrue(iplist.size() == 1);
            assertArrayEquals(sender, iplist.iterator().next().getAddress());

            // check table size
            List<MacAddressEntry> list = null;
            try {
                list = tbl.getEntries();
            } catch (Exception e) {
                unexpected(e);
            }
            assertNotNull(list);
            assertTrue(list.size() == 1);
            EthernetAddress eth = (EthernetAddress) list.get(0).getAddress();
            assertArrayEquals(src, eth.getValue());

            // remove entry
            tbl.remove(rpctx);
            assertNull(tbl.get(rpctx));

            // check whether removed entry not included.
            tent = tbl.get(rpctx);
            assertNull(tent);

            // removeEntry()
            tbl.add(mgr, dpctx);
            try {
                mae = tbl.removeEntry(ea);
            } catch (Exception e) {
                unexpected(e);
            }
            assertNotNull(mae);
            assertEquals(ea, mae.getAddress());
            assertEquals(0, mae.getVlan());
            assertEquals(connectors.get(0), mae.getNodeConnector());

            try {
                mae = tbl.removeEntry(ea);
            } catch (Exception e) {
                unexpected(e);
            }
            assertNull(mae);

            // check whether removed entry not included.
            tent = tbl.get(rpctx);
            assertNull(tent);

            tall.add(mgr, dpctx);
            list = null;
            try {
                list = tall.getEntries();
            } catch (Exception e) {
                unexpected(e);
            }
            assertNotNull(list);
            assertTrue(list.size() == iphost);

            iphost++;
        }
        tbl.destroy(mgr);
        tall.destroy(mgr);

        // case for Multicast packet data
        byte [] dst = new byte[] {(byte)0xFF, (byte)0xFF, (byte)0xFF,
                                    (byte)0xFF, (byte)0xFF, (byte)0xFF};
        byte [] src = new byte[] {(byte)0xFF, (byte)0x00, (byte)0x00,
                                    (byte)0x00, (byte)0x00, (byte)0x01};
        byte [] sender = new byte[] {(byte)192, (byte)168, (byte)100, (byte)1};
        byte [] target = new byte[] {(byte)192, (byte)168, (byte)100, (byte)2};

        MacAddressTable mtbl = new MacAddressTable(mgr, "Table", 600);
        PacketContext mpctx = createARPPacketContext(src, dst, sender, target,
                                           (short)-1, connectors.get(0), ARP.REQUEST);
        mtbl.add(mgr, mpctx);

        List<MacAddressEntry> list = null;
        try {
            list = mtbl.getEntries();
        } catch (Exception e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertTrue(list.size() == 0);
    }

    /**
     * Test case for
     * {@link MacAddressTable#flush()},
     * {@link MacAddressTable#flush(Node)},
     * {@link MacAddressTable#flush(Node, short)}
     */
    @Test
    public void testFlush() {
        VTNManagerImpl mgr = vtnMgr;

        MacAddressTable tbl1 = new MacAddressTable(mgr, "Table", 600);
        MacAddressTable tbl2 = new MacAddressTable(mgr, "Table2", 1000);
        MacAddressTable tbl3 = new MacAddressTable(mgr, "Table3", 1000);
        MacAddressTable tbl4 = new MacAddressTable(mgr, "Table4", 1000);
        MacAddressTable tbl5 = new MacAddressTable(mgr, "Table5", 1000);

        List<NodeConnector> connectors = createNodeConnectors(3, false);
        short vlan = 0;
        for (byte iphost = 1; iphost <= 9; iphost++) {
            byte[] src = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                        (byte)0x00, (byte)0x00, (byte)iphost};
            byte[] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                        (byte)0xff, (byte)0xff, (byte)0xff};
            byte[] sender = new byte[] {(byte)192, (byte)168, (byte)100, (byte)iphost};
            byte[] target = new byte[] {(byte)192, (byte)168, (byte)100, (byte)250};

            PacketContext pctx = createARPPacketContext(src, dst, sender, target, (short)(vlan / 3),
                                               connectors.get(iphost % 3), ARP.REQUEST);
            tbl1.add(mgr, pctx);
            tbl2.add(mgr, pctx);
            tbl3.add(mgr, pctx);
            tbl4.add(mgr, pctx);
            tbl5.add(mgr, pctx);

            vlan++;
        }

        List<MacAddressEntry> list = null;
        tbl1.flush();
        try {
            list = tbl1.getEntries();
        } catch (Exception e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertTrue(list.size() == 0);

        tbl2.flush(connectors.get(0).getNode());
        try {
            list = tbl2.getEntries();
        } catch (Exception e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertTrue(list.size() == 6);

        tbl3.flush(connectors.get(0).getNode(), (short)0);
        try {
            list = tbl3.getEntries();
        } catch (Exception e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertTrue(list.size() == 8);

        tbl4.flush(connectors.get(0));
        try {
            list = tbl4.getEntries();
        } catch (Exception e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertTrue(list.size() == 6);

        tbl5.flush(connectors.get(0), (short)0);
        try {
            list = tbl5.getEntries();
        } catch (Exception e) {
            unexpected(e);
        }
        assertNotNull(list);
        assertTrue(list.size() == 8);
    }
}
