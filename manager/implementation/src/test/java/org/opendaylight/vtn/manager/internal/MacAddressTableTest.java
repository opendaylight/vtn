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
import java.util.HashSet;
import java.util.Hashtable;
import java.util.List;
import java.util.Set;
import java.util.Timer;
import java.util.TimerTask;

import org.apache.felix.dm.impl.ComponentImpl;
import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntryId;

/**
 * JUnit test for {@link MacAddressTable}
 */
public class MacAddressTableTest extends TestBase {
    private VTNManagerImpl vtnMgr = null;
    private GlobalResourceManager resMgr;

    // The Test class which implemented DataLinkAddress class.
    class TestDataLink extends DataLinkAddress {
        private static final long serialVersionUID = 5664547077536394233L;

        TestDataLink() {

        }

        TestDataLink(String name) {
            super(name);
        }

        public TestDataLink clone() {
            return new TestDataLink(this.getName());
        }
    }

    /**
     * setup a test environment for MacAddressTableTest
     */
    @Before
    public void before() {
        vtnMgr = new VTNManagerImpl();
        resMgr = new GlobalResourceManager();
        ComponentImpl c = new ComponentImpl(null, null, null);

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        resMgr.init(c);
        vtnMgr.setResourceManager(resMgr);
        vtnMgr.init(c);
    }

    /**
     * cleanup a test environment
     */
    @After
    public void after() {
        vtnMgr.stopping();
        vtnMgr.stop();
        vtnMgr.destroy();

        resMgr.destroy();
    }

    /**
     * Test case for {@link MacAddressTable#getTableKey(byte [])}
     */
    @Test
    public void testGetTableKey() {
        for (EthernetAddress ea: createEthernetAddresses(false)) {
            String emsg = ea.toString();
            byte [] value = ea.getValue();
            Long lvalue = MacAddressTable.getTableKey(ea.getValue());

            int len = value.length;
            for (int i = 0; i < len; i++) {
                assertEquals(emsg, value[len - i - 1],
                        (byte)((lvalue.longValue() >> (i * 8)) & 0xff));
            }
            assertTrue(emsg, lvalue.longValue() < (long)(1L << (len * 8)));
        }
    }

    /**
     * Test case for
     * {@link MacAddressTable#MacAddressTable(VTNManagerImpl, String, int)} and
     * {@link MacAddressTable#destroy()}
     */
    @Test
    public void testMacAddressTable() {
        VTNManagerImpl mgr = vtnMgr;

        VBridgePath path = new VBridgePath("tenant1", "bridge1");
        MacAddressTable tbl = new MacAddressTable(mgr, path, 600);
        assertNotNull(tbl);
        tbl.destroy(true);

        tbl = new MacAddressTable(mgr, path, 10);
        assertNotNull(tbl);
        tbl.destroy(true);

        tbl = new MacAddressTable(mgr, path, 1000000);
        assertNotNull(tbl);
        tbl.destroy(true);
    }

    /**
     * Test case for {@link MacAddressTable#setAgeInterval(VTNManagerImpl, int)}
     */
    @Test
    public void testSetAgeInterval() {
        VTNManagerImpl mgr = vtnMgr;

        VBridgePath path = new VBridgePath("tenant1", "bridge1");
        MacAddressTable tbl = new MacAddressTable(mgr, path, 600);
        tbl.setAgeInterval(10);
        tbl.setAgeInterval(1000000);
        tbl.setAgeInterval(1000000);
        tbl.destroy(true);
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
        VBridgePath path1 = new VBridgePath("tenant1", "bridge1");
        VBridgePath path2 = new VBridgePath("tenant1", "bridge2");
        MacAddressTable tbl = new MacAddressTable(mgr, path1, 600);
        MacAddressTable tall = new MacAddressTable(mgr, path2, 600);
        MacTableEntry tent = null;
        MacAddressEntry mae = null;

        byte iphost = 1;
        short vlan = 4095;
        byte [] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                  (byte)0xff, (byte)0xff, (byte)0xff};
        byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

        List<NodeConnector> connectors = createNodeConnectors(3, false);

        for (EthernetAddress ea: createEthernetAddresses(false)) {
            String emsg = ea.toString();

            byte [] bytes = ea.getValue();
            byte [] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                      bytes[3], bytes[4], bytes[5]};
            byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)iphost};
            byte [] sender2
                = new byte[] {(byte)192, (byte)168, (byte)0, (byte)(iphost + 100)};

            // packet context added to table at first
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
            tent = tbl.get(getTableKey(dpctx));
            assertNull(emsg, tent);
            mae = getEntry(tbl, ea);
            assertNull(emsg, mae);

            // add() and get() one data
            tbl.add(dpctx);
            tent = tbl.get(getTableKey(dpctx));
            assertNull(emsg, tent);
            tent = tbl.get(getTableKey(rpctx));
            assertNotNull(emsg, tent);
            assertTrue(emsg, tent.clearUsed());
            assertEquals(emsg, connectors.get(0), tent.getPort());
            assertEquals(emsg, 0, tent.getVlan());

            // getEntry()
            mae = getEntry(tbl, ea);
            assertNotNull(emsg, mae);
            assertEquals(emsg, ea, mae.getAddress());
            assertEquals(emsg, 0, mae.getVlan());
            assertEquals(emsg, connectors.get(0), mae.getNodeConnector());
            Set<InetAddress> iplist = mae.getInetAddresses();
            assertEquals(emsg, 1, iplist.size());
            assertArrayEquals(emsg, sender, iplist.iterator().next().getAddress());

            // add same packet
            assertFalse(emsg, tent.clearUsed());
            tbl.add(dpctx);
            assertTrue(emsg, tent.clearUsed());

            // add packet IP address changed
            tbl.add(ippctx);
            tent = tbl.get(getTableKey(rpctx));
            assertNotNull(emsg, tent);
            assertEquals(emsg, connectors.get(0), tent.getPort());
            assertEquals(emsg, 0, tent.getVlan());
            mae = getEntry(tbl, ea);
            iplist = mae.getInetAddresses();
            assertEquals(emsg, 2, iplist.size());
            InetAddress ip = null, ip2 = null;
            try {
                ip = InetAddress.getByAddress(sender);
                ip2 = InetAddress.getByAddress(sender2);
            } catch (UnknownHostException e) {
                unexpected(e);
            }
            assertTrue(emsg, iplist.contains(ip));
            assertTrue(emsg, iplist.contains(ip2));
            assertFalse(emsg, ip.equals(ip2));

            tbl.add(dpctx);

            // add packet vlan changed
            tbl.add(vlanpctx);
            tent = tbl.get(getTableKey(rpctx));
            assertNotNull(emsg, tent);
            assertEquals(emsg, connectors.get(0), tent.getPort());
            assertEquals(emsg, vlan, tent.getVlan());
            mae = getEntry(tbl, ea);
            iplist = mae.getInetAddresses();
            assertEquals(emsg, 1, iplist.size());
            assertArrayEquals(emsg, sender, iplist.iterator().next().getAddress());

            tbl.add(dpctx);

            // add packet set different nodeconnector
            tbl.add(ncpctx);
            tent = tbl.get(getTableKey(rpctx));
            assertNotNull(emsg, tent);
            assertEquals(emsg, connectors.get(1), tent.getPort());
            assertEquals(emsg, 0, tent.getVlan());
            mae = getEntry(tbl, ea);
            iplist = mae.getInetAddresses();
            assertEquals(emsg, 1, iplist.size());
            assertArrayEquals(emsg, sender, iplist.iterator().next().getAddress());

            // check table size
            List<MacAddressEntry> list = null;
            list = getEntries(tbl);
            assertNotNull(emsg, list);
            assertEquals(emsg, 1, list.size());
            EthernetAddress eth = (EthernetAddress) list.get(0).getAddress();
            assertArrayEquals(emsg, src, eth.getValue());

            // remove entry
            tbl.remove(getTableKey(rpctx));
            assertNull(emsg, tbl.get(getTableKey(rpctx)));

            // check whether removed entry not included.
            tent = tbl.get(getTableKey(rpctx));
            assertNull(emsg, tent);

            // remove entry again.
            tbl.remove(getTableKey(rpctx));
            assertNull(emsg, tbl.get(getTableKey(rpctx)));

            // check whether removed entry not included.
            tent = tbl.get(getTableKey(rpctx));
            assertNull(emsg, tent);


            // removeEntry()
            tbl.add(dpctx);
            try {
                mae = tbl.removeEntry(ea);
            } catch (Exception e) {
                unexpected(e);
            }
            assertNotNull(emsg, mae);
            assertEquals(emsg, ea, mae.getAddress());
            assertEquals(emsg, 0, mae.getVlan());
            assertEquals(emsg, connectors.get(0), mae.getNodeConnector());

            try {
                mae = tbl.removeEntry(ea);
            } catch (Exception e) {
                unexpected(e);
            }
            assertNull(emsg, mae);

            // check whether removed entry not included.
            tent = tbl.get(getTableKey(rpctx));
            assertNull(emsg, tent);

            tall.add(dpctx);
            list = getEntries(tall);
            assertNotNull(emsg, list);
            assertEquals(emsg, iphost, list.size());

            iphost++;
        }

        tbl.destroy(true);
        tall.destroy(true);

        // call each method after call destroy().
        byte [] src = new byte[] {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
        PacketContext pctx = createARPPacketContext(src, dst, sender, target,
                                        (short)-1, connectors.get(0), ARP.REQUEST);
        tbl.add(pctx);
        InetAddress ipaddr = null;
        try {
            ipaddr = InetAddress.getByAddress(sender);
        } catch (UnknownHostException e) {
            unexpected(e);
        }

        long key = 1000L;
        tent = new MacTableEntry(path1, key, connectors.get(0), (short) 0, ipaddr);

        tbl.add(tent);
        assertNull(tbl.get(key));

        List<MacAddressEntry> entries = getEntries(tbl);
        assertEquals(0, entries.size());

        EthernetAddress ea = null;
        try {
            ea = new EthernetAddress(src);
        } catch (ConstructionException e) {
            unexpected(e);
        }
        assertNull(getEntry(tbl, ea));

        mae = null;
        try {
            mae = tbl.removeEntry(ea);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNull(mae);

        // check run normally in this condition.
        tbl.remove(key);
        tbl.destroy(true);

        // case for a multicast packet data
        src = new byte[] {(byte)0xFF, (byte)0x00, (byte)0x00,
                          (byte)0x00, (byte)0x00, (byte)0x01};
        sender = new byte[] {(byte)192, (byte)168, (byte)100, (byte)1};

        VBridgePath path = new VBridgePath("tenant1", "bridge1");
        MacAddressTable mtbl = new MacAddressTable(mgr, path, 600);
        PacketContext mpctx = createARPPacketContext(src, dst, sender, target,
                                           (short)-1, connectors.get(0), ARP.REQUEST);
        mtbl.add(mpctx);

        List<MacAddressEntry> list = null;
        list = getEntries(mtbl);
        assertNotNull(list);
        assertEquals(0, list.size());

        // query by bad object.
        TestDataLink dl = new TestDataLink("test");

        try {
            mae = mtbl.getEntry(dl);
        } catch (VTNException e) {
            unexpected(e);
        }
        assertNull(mae);

        try {
            mae = mtbl.removeEntry(dl);
        } catch (VTNException e) {
           unexpected(e);
        }
        assertNull(mae);

        mtbl.destroy(true);
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

        VBridgePath path1 = new VBridgePath("tenant1", "bridge1");
        VBridgePath path2 = new VBridgePath("tenant1", "bridge2");
        VBridgePath path3 = new VBridgePath("tenant1", "bridge3");
        VBridgePath path4 = new VBridgePath("tenant1", "bridge4");
        VBridgePath path5 = new VBridgePath("tenant1", "bridge5");
        VBridgePath path6 = new VBridgePath("tenant1", "bridge6");
        VBridgePath path7 = new VBridgePath("tenant1", "bridge7");
        MacAddressTable tbl1 = new MacAddressTable(mgr, path1, 600);
        MacAddressTable tbl2 = new MacAddressTable(mgr, path2, 1000);
        MacAddressTable tbl3 = new MacAddressTable(mgr, path3, 1000);
        MacAddressTable tbl4 = new MacAddressTable(mgr, path4, 1000);
        MacAddressTable tbl5 = new MacAddressTable(mgr, path5, 1000);
        MacAddressTable tbl6 = new MacAddressTable(mgr, path6, 1000);
        MacAddressTable tbl7 = new MacAddressTable(mgr, path7, 1000);

        InetAddress contIpAddr1 = null;
        InetAddress contIpAddr2 = null;
        try {
            contIpAddr1
                = InetAddress.getByAddress(new byte[] {(byte)192, (byte)168,
                                                       (byte)100, (byte)254});
            contIpAddr2
                = InetAddress.getByAddress(new byte[] {(byte)192, (byte)168,
                                                       (byte)100, (byte)253});
        } catch (UnknownHostException e) {
            unexpected(e);
        }

        List<NodeConnector> connectors = createNodeConnectors(3, false);
        short vlan = 0;
        byte[] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                 (byte)0xff, (byte)0xff, (byte)0xff};
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)100, (byte)250};

        int id = 1000;
        for (byte iphost = 1; iphost <= 9; iphost++) {
            byte[] src = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                     (byte)0x00, (byte)0x00, (byte)iphost};
            byte[] sender = new byte[] {(byte)192, (byte)168, (byte)100, (byte)iphost};

            PacketContext pctx = createARPPacketContext(src, dst, sender, target,
                                               (short)(((vlan / 3) > 0) ? (vlan / 3) : -1),
                                               connectors.get(iphost % 3), ARP.REQUEST);
            tbl1.add(pctx);
            tbl2.add(pctx);
            tbl3.add(pctx);
            tbl4.add(pctx);
            tbl5.add(pctx);
            tbl6.add(pctx);

            InetAddress ipaddr = null;
            try {
                ipaddr = InetAddress.getByAddress(sender);
            } catch (UnknownHostException e) {
                unexpected(e);
            }
            MacTableEntryId mentid
                    = new MacTableEntryId(((id % 2) == 0) ? contIpAddr1 : contIpAddr2,
                                          (long) id, path7,
                                          NetUtils.byteArray6ToLong(src));
            MacTableEntry tent
                    = new MacTableEntry(mentid, connectors.get(iphost % 3),
                                        (short) (((vlan / 3) > 0) ? (vlan / 3) : -1),
                                        ipaddr);
            tbl7.add(tent);

            id++;
            vlan++;
        }

        // flush all entries.
        List<MacAddressEntry> list = null;
        tbl1.flush();
        list = getEntries(tbl1);
        assertNotNull(list);
        assertEquals(0, list.size());

        // flush entry relevant to specified Node.
        NodeConnector nc = connectors.get(0);
        tbl2.flush(nc.getNode());
        list = getEntries(tbl2);
        assertNotNull(list);
        assertEquals(6, list.size());

        // flush entry relevant to specified Node and vlan id.
        tbl3.flush(nc.getNode(), (short)0);
        list = getEntries(tbl3);
        assertNotNull(list);
        assertEquals(8, list.size());

        // flush entry relevant to specified NodeConnector.
        tbl4.flush(nc);
        list = getEntries(tbl4);
        assertNotNull(list);
        assertEquals(6, list.size());

        // flush entry relevant to specified NodeConnector and vlan id.
        tbl5.flush(nc, (short)0);
        list = getEntries(tbl5);
        assertNotNull(list);
        assertEquals(8, list.size());

        // flush entry relevant to specified NodeConnector and vlan id.
        tbl6.flush((Node)null, (short)0);
        list = getEntries(tbl6);
        assertNotNull(list);
        assertEquals(6, list.size());

        // flush entry relevant to specified controller IP address.
        Set<InetAddress> addrs = new HashSet<InetAddress>();
        addrs.add(contIpAddr1);
        tbl7.flush(addrs);
        list = getEntries(tbl7);
        assertNotNull(list);
        assertEquals(4, list.size());

        // check execute normally after call destroy().
        tbl1.destroy(true);

        nc = connectors.get(0);
        tbl1.flush(nc.getNode());
        tbl1.flush(nc.getNode(), (short) 0);
        tbl1.flush(nc);
        tbl1.flush(nc, (short) 0);
        tbl1.flush(new HashSet<InetAddress>());
        tbl1.flush();
    }

    /**
     * Test case for {@link entryUpdate() and entryDeleted()}
     */
    @Test
    public void testEntryUpdatedAndDeleted() {
        VTNManagerImpl mgr = vtnMgr;
        VBridgePath path = new VBridgePath("tenant1", "bridge1");
        MacAddressTable tbl = new MacAddressTable(mgr, path, 600);

        InetAddress contIpAddr = null;
        try {
            contIpAddr
                = InetAddress.getByAddress(new byte[] {(byte)192, (byte)168,
                                                       (byte)100, (byte)254});
        } catch (UnknownHostException e) {
            unexpected(e);
        }

        List<NodeConnector> connectors = createNodeConnectors(3, false);
        short vlan = 0;

        int numEntries = 9;
        for (byte iphost = 1; iphost <= numEntries; iphost++) {
            byte[] src = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                     (byte)0x00, (byte)0x00, (byte)iphost};
            byte[] sender = new byte[] {(byte)192, (byte)168, (byte)100, (byte)iphost};

            long key = NetUtils.byteArray6ToLong(src);
            InetAddress ipaddr = null;
            try {
                ipaddr = InetAddress.getByAddress(sender);
            } catch (UnknownHostException e) {
                unexpected(e);
            }
            MacTableEntry tent
                    = new MacTableEntry(path, key, connectors.get(iphost % 3),
                                        (short) (((vlan / 3) > 0) ? (vlan / 3) : -1),
                                        ipaddr);
            InetAddress ipaddr2 = null;
            try {
                ipaddr = InetAddress.getByAddress(sender);
            } catch (UnknownHostException e) {
                unexpected(e);
            }
            MacTableEntry tent2
                    = new MacTableEntry(path, key, connectors.get(iphost % 3),
                                (short) (((vlan / 3) > 0) ? (vlan / 3) : -1),
                                ipaddr2);
            tbl.add(tent);

            // add entry
            tbl.entryUpdated(tent);
            MacTableEntry gent = tbl.get(key);
            assertEquals(tent, gent);

            // overwrite entry.
            tbl.entryUpdated(tent2);
            gent = tbl.get(key);
            assertEquals(tent2, gent);

            // in case unmatched mac address is specified.
            MacTableEntryId mentid = new MacTableEntryId(contIpAddr, 1000L, path, 0L);
            tbl.entryDeleted(mentid);
            assertEquals(tent2, gent);

            // in case mac address match but entryID doesn't match.
            mentid = new MacTableEntryId(contIpAddr, 1000L, path,
                                         NetUtils.byteArray6ToLong(src));
            tbl.entryDeleted(mentid);
            assertEquals(tent2, gent);

            // delete entry.
            tbl.entryDeleted(tent2.getEntryId());
            gent = tbl.get(key);
            assertNull(gent);

            // add entry again.
            tbl.entryUpdated(tent);
            gent = tbl.get(key);
            assertEquals(tent, gent);

            vlan++;
        }

        List<MacAddressEntry> entries = getEntries(tbl);
        assertEquals(numEntries, entries.size());

        // check execute normally after call destroy().
        tbl.destroy(true);
        MacTableEntry tent = new MacTableEntry(path, 0L, connectors.get(0),
                                               (short) -1, contIpAddr);
        tbl.entryUpdated(tent);
        tbl.entryDeleted(tent.getEntryId());
    }

    /**
     * Test case for {@link age())}
     */
    @Test
    public void testAge() {

        // Task used to put a table entry in parallel.
        class PutTableTask extends TimerTask {
            MacAddressTable tbl;
            PacketContext pctx;

            PutTableTask(MacAddressTable tbl, PacketContext pctx) {
                this.tbl = tbl;
                this.pctx = pctx;
            }

            @Override
            public void run() {
                tbl.add(pctx);
            }
        }

        VTNManagerImpl mgr = vtnMgr;
        VBridgePath path = new VBridgePath("tenant1", "bridge1");
        MacAddressTable tbl = new MacAddressTable(mgr, path, 1);

        short vlan = 4095;
        byte [] src = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                  (byte)0xff, (byte)0xff, (byte)0x11};
        byte [] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                        (byte)0xff, (byte)0xff, (byte)0xff};
        byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
        byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
        Node node = NodeCreator.createOFNode(0L);
        NodeConnector nc = NodeConnectorCreator.createOFNodeConnector((short)0, node);

        // packet context added to table at first.
        PacketContext pctx = createARPPacketContext(src, dst, sender, target,
                                                       (vlan > 0) ? vlan : -1, nc, ARP.REQUEST);
        EthernetAddress ea = null;
        try {
            ea = new EthernetAddress(src);
        } catch (ConstructionException e) {
            unexpected(e);
        }

        tbl.add(pctx);
        MacAddressEntry mae = getEntry(tbl, ea);
        assertNotNull(mae);
        assertEquals(ea, mae.getAddress());

        sleep(2500);

        // expect the entry to be removed by aging task.
        mae = getEntry(tbl, ea);
        assertNull(mae);

        Timer timer = new Timer();
        PutTableTask ptask = new PutTableTask(tbl, pctx);
        timer.schedule(ptask, 0, 500L);

        tbl.add(pctx);

        sleep(2500);

        // expect the entry to exist
        mae = getEntry(tbl, ea);
        assertNotNull(mae);
        assertEquals(ea, mae.getAddress());

        timer.cancel();
        timer.purge();
    }


    // private methods

    /**
     * get Mac address table entries
     */
    private List<MacAddressEntry> getEntries(MacAddressTable tbl) {
        List<MacAddressEntry> list = null;
        try {
            list = tbl.getEntries();
        } catch (Exception e) {
            unexpected(e);
        }
        return list;
    }

    /**
     * get Mac address table entry
     */
    private MacAddressEntry getEntry(MacAddressTable tbl, EthernetAddress ea) {
        MacAddressEntry mae = null;
        try {
            mae = tbl.getEntry(ea);
        } catch (VTNException e) {
            unexpected(e);
        }
        return mae;
    }

    /**
     * Return a {@code Long} object which represents the destination address
     * of the given packet context.
     *
     * @param pctx  The packet context.
     * @return  A {@code Long} object which represents the destination address
     *          of the given packet context.
     */
    private Long getTableKey(PacketContext pctx) {
        byte[] dst = pctx.getDestinationAddress();
        return MacAddressTable.getTableKey(dst);
    }
}
