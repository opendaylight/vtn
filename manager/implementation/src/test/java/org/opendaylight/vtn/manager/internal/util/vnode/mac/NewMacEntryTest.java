/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode.mac;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.vnode.MacMapIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VBridgeIfIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;
import org.opendaylight.vtn.manager.internal.util.vnode.VlanMapIdentifier;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpAddress;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Address;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * JUnit test for {@link NewMacEntryTest}.
 */
public class NewMacEntryTest extends TestBase {
    /**
     * Test case for {@link MacEntry#getEtherAddress()}.
     */
    @Test
    public void testGetEtherAddress() {
        SalPort sport = new SalPort(1L, 2L);
        String pname = "port-2";
        int vid = 0;
        IpNetwork ip = null;
        VNodeIdentifier<?> ident = new MacMapIdentifier(
            new VnodeName("vtn_1"), new VnodeName("vbr_1"));

        EtherAddress[] eaddrs = {
            new EtherAddress(0x001122334455L),
            new EtherAddress(0x000000000001L),
            new EtherAddress(0xfafbfcfdfeffL),
        };

        for (EtherAddress eaddr: eaddrs) {
            MacEntry ment =
                new NewMacEntry(eaddr, sport, pname, vid, ip, ident);
            assertEquals(eaddr, ment.getEtherAddress());
        }
    }

    /**
     * Test case for {@link MacEntry#getPort()}.
     */
    @Test
    public void testGetPort() {
        EtherAddress eaddr = new EtherAddress(0x001122334455L);
        String pname = "port-2";
        int vid = 0;
        IpNetwork ip = null;
        VNodeIdentifier<?> ident = new MacMapIdentifier(
            new VnodeName("vtn_1"), new VnodeName("vbr_1"));

        SalPort[] ports = {
            new SalPort(1L, 2L),
            new SalPort(123L, 456L),
            new SalPort(-1L, 789L),
        };

        for (SalPort sport: ports) {
            MacEntry ment =
                new NewMacEntry(eaddr, sport, pname, vid, ip, ident);
            assertEquals(sport, ment.getPort());
        }
    }

    /**
     * Test case for {@link MacEntry#getVlanId()}.
     */
    @Test
    public void testGetVlanId() {
        EtherAddress eaddr = new EtherAddress(0x001122334455L);
        SalPort sport = new SalPort(1L, 2L);
        String pname = "port-2";
        IpNetwork ip = null;
        VNodeIdentifier<?> ident = new MacMapIdentifier(
            new VnodeName("vtn_1"), new VnodeName("vbr_1"));

        int[] vids = {0, 1, 123, 1234, 4000, 4095};

        for (int vid: vids) {
            MacEntry ment =
                new NewMacEntry(eaddr, sport, pname, vid, ip, ident);
            assertEquals(vid, ment.getVlanId());
        }
    }

    /**
     * Test case for {@link MacEntry#hasMoved(SalPort, int, VNodeIdentifier)}.
     */
    @Test
    public void testHasMoved() {
        EtherAddress eaddr = new EtherAddress(0x001122334455L);
        SalPort targetPort = new SalPort(1L, 2L);
        String pname = "port-2";
        int targetVid = 0;
        IpNetwork ip = null;
        VnodeName vtnName = new VnodeName("vtn_1");
        VnodeName vbrName = new VnodeName("vbr_1");
        VNodeIdentifier<?> targetIdent =
            new MacMapIdentifier(vtnName, vbrName);
        MacEntry ment = new NewMacEntry(eaddr, targetPort, pname, targetVid,
                                        ip, targetIdent);

        SalPort[] ports = {
            targetPort,
            new SalPort(1L, 3L),
            new SalPort(2L, 2L),
        };
        int[] vids = {targetVid, targetVid + 3, targetVid + 100};
        List<VNodeIdentifier<?>> identifiers = new ArrayList<>();
        Collections.addAll(
            identifiers, targetIdent,
            new VlanMapIdentifier(vtnName, vbrName, "ANY.0"),
            new VBridgeIfIdentifier(vtnName, vbrName, new VnodeName("if_1")));

        for (SalPort sport: ports) {
            for (int vid: vids) {
                for (VNodeIdentifier<?> ident: identifiers) {
                    boolean expected = (!sport.equals(targetPort) ||
                                        vid != targetVid ||
                                        !ident.equals(targetIdent));
                    assertEquals(expected, ment.hasMoved(sport, vid, ident));
                }
            }
        }
    }

    /**
     * Test case for {@link MacEntry#needIpProbe()}.
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testIpProbe() throws Exception {
        EtherAddress eaddr = new EtherAddress(0x001122334455L);
        SalPort sport = new SalPort(1L, 2L);
        String pname = "port-2";
        int vid = 0;
        VNodeIdentifier<?> ident = new MacMapIdentifier(
            new VnodeName("vtn_1"), new VnodeName("vbr_1"));

        // In case where an IP address is specified.
        IpNetwork ip = new Ip4Network("192.168.100.200");
        MacEntry ment = new NewMacEntry(eaddr, sport, pname, vid, ip, ident);
        assertEquals(false, ment.needIpProbe());
        Integer cnt = getFieldValue(ment, MacEntry.class, Integer.class,
                                    "ipProbe");
        assertEquals(0, cnt.intValue());

        // In case where no IP address is specified.
        ip = null;
        ment = new NewMacEntry(eaddr, sport, pname, vid, ip, ident);
        for (int i = 0; i <= 20; i++) {
            int expected;
            if (i < 10) {
                assertEquals(true, ment.needIpProbe());
                expected = i + 1;
            } else {
                assertEquals(false, ment.needIpProbe());
                expected = 10;
            }

            cnt = getFieldValue(ment, MacEntry.class, Integer.class,
                                "ipProbe");
            assertEquals(expected, cnt.intValue());
        }
    }

    /**
     * Test case for {@link MacEntry#getNewEntry()}.
     */
    @Test
    public void testGetNewEntry() {
        MacAddress mac = new MacAddress("00:11:22:33:44:55");
        EtherAddress eaddr = new EtherAddress(mac);
        long dpid = 123;
        long pnum = 45;
        SalPort sport = new SalPort(dpid, pnum);
        NodeId nid = new NodeId("openflow:" + dpid);
        String pname = "port-2";
        int vid = 0;
        VNodeIdentifier<?> ident = new MacMapIdentifier(
            new VnodeName("vtn_1"), new VnodeName("vbr_1"));

        // In case where an IP address is specified.
        String ip = "192.168.100.200";
        List<IpAddress> ipaddrs = new ArrayList<>();
        ipaddrs.add(new IpAddress(new Ipv4Address(ip)));
        IpNetwork ipn = new Ip4Network(ip);
        MacEntry ment = new NewMacEntry(eaddr, sport, pname, vid, ipn, ident);
        MacTableEntry mtent = ment.getNewEntry();

        assertEquals(mac, mtent.getMacAddress());
        assertEquals(nid, mtent.getNode());
        assertEquals(String.valueOf(pnum), mtent.getPortId());
        assertEquals(pname, mtent.getPortName());
        assertEquals(vid, mtent.getVlanId().intValue());
        assertEquals(ident, VNodeIdentifier.create(mtent.getEntryData()));
        assertEquals(0, mtent.getIpProbeCount().intValue());
        assertEquals(ipaddrs, mtent.getIpAddresses());

        // In case where no IP address is specified.
        ipn = null;
        ipaddrs = null;
        ment = new NewMacEntry(eaddr, sport, pname, vid, ipn, ident);
        for (int i = 0; i <= 20; i++) {
            int probes;
            if (i < 10) {
                assertEquals(true, ment.needIpProbe());
                probes = i + 1;
            } else {
                assertEquals(false, ment.needIpProbe());
                probes = 10;
            }

            mtent = ment.getNewEntry();
            assertEquals(mac, mtent.getMacAddress());
            assertEquals(nid, mtent.getNode());
            assertEquals(String.valueOf(pnum), mtent.getPortId());
            assertEquals(pname, mtent.getPortName());
            assertEquals(vid, mtent.getVlanId().intValue());
            assertEquals(ident, VNodeIdentifier.create(mtent.getEntryData()));
            assertEquals(probes, mtent.getIpProbeCount().intValue());
            assertEquals(ipaddrs, mtent.getIpAddresses());
        }
    }

    /**
     * Test case for {@link MacEntry#toString()}.
     */
    @Test
    public void testToString() {
        String[] macs = {
            "00:11:22:33:44:55",
            "fa:bc:de:f0:12:34",
        };
        String[] ports = {
            "openflow:1:2",
            "openflow:123:456",
        };
        String pname = "port-2";
        IpNetwork[] ipaddrs = {
            null,
            new Ip4Network("192.168.1.2"),
            new Ip4Network("10.20.30.40"),
        };
        int[] vids = {0, 123, 4095};

        List<VNodeIdentifier<?>> identifiers = new ArrayList<>();
        Collections.addAll(
            identifiers,
            new MacMapIdentifier(new VnodeName("vtn_1"),
                                 new VnodeName("vbr_1")),
            new VlanMapIdentifier(new VnodeName("vtn_2"),
                                  new VnodeName("vbr_2"), "ANY.0"));

        for (String mac: macs) {
            EtherAddress eaddr = new EtherAddress(mac);
            for (String port: ports) {
                SalPort sport = SalPort.create(port);
                for (int vid: vids) {
                    for (IpNetwork ipn: ipaddrs) {
                        String ipStr;
                        if (ipn == null) {
                            ipStr = "";
                        } else {
                            Set<IpNetwork> ipSet = Collections.singleton(ipn);
                            ipStr = ", ipaddr=" + ipSet;
                        }
                        for (VNodeIdentifier<?> ident: identifiers) {
                            MacEntry ment = new NewMacEntry(
                                eaddr, sport, pname, vid, ipn, ident);
                            String expected = "mac-table-entry[mac=" + mac +
                                ", port=" + port + ", vid=" + vid +
                                ", mapPath=" + ident.toString() + ipStr +
                                "]";
                            assertEquals(expected, ment.toString());
                        }
                    }
                }
            }
        }
    }

    /**
     * Test case for {@link NewMacEntry#addIpAddress(IpNetwork)}.
     */
    @Test
    public void testAddIpAddress() {
        EtherAddress eaddr = new EtherAddress(0x001122334455L);
        SalPort sport = new SalPort(1L, 2L);
        String pname = "port-2";
        int vid = 0;
        IpNetwork ip = null;
        VNodeIdentifier<?> ident = new MacMapIdentifier(
            new VnodeName("vtn_1"), new VnodeName("vbr_1"));

        MacEntry ment = new NewMacEntry(eaddr, sport, pname, vid, ip, ident);
        ip = new Ip4Network("10.20.30.40");
        try {
            ment.addIpAddress(ip);
            unexpected();
        } catch (IllegalStateException e) {
        }
    }

    /**
     * Test case for {@link NewMacEntry#getIpAddresses()}.
     */
    @Test
    public void testGetIpAddresses() {
        EtherAddress eaddr = new EtherAddress(0x001122334455L);
        SalPort sport = new SalPort(1L, 2L);
        String pname = "port-2";
        int vid = 0;
        VNodeIdentifier<?> ident = new MacMapIdentifier(
            new VnodeName("vtn_1"), new VnodeName("vbr_1"));

        String[] ipaddrs = {
            null,
            "10.20.30.40",
            "192.168.1.2",
            "190.32.45.254",
        };

        for (String ip: ipaddrs) {
            IpNetwork ipn;
            List<IpAddress> expected;
            if (ip == null) {
                ipn = null;
                expected = null;
            } else {
                ipn = new Ip4Network(ip);
                expected = new ArrayList<>();
                expected.add(new IpAddress(new Ipv4Address(ip)));
            }

            MacEntry ment =
                new NewMacEntry(eaddr, sport, pname, vid, ipn, ident);
            assertEquals(expected, ment.getIpAddresses());
        }
    }

    /**
     * Test case for {@link NewMacEntry#getIpNetworkSet()}.
     */
    @Test
    public void testGetIpNetworkSet() {
        EtherAddress eaddr = new EtherAddress(0x001122334455L);
        SalPort sport = new SalPort(1L, 2L);
        String pname = "port-2";
        int vid = 0;
        VNodeIdentifier<?> ident = new MacMapIdentifier(
            new VnodeName("vtn_1"), new VnodeName("vbr_1"));

        String[] ipaddrs = {
            null,
            "10.20.30.40",
            "192.168.1.2",
            "190.32.45.254",
        };

        for (String ip: ipaddrs) {
            IpNetwork ipn;
            Set<IpNetwork> expected = new HashSet<>();
            if (ip == null) {
                ipn = null;
            } else {
                ipn = new Ip4Network(ip);
                assertTrue(expected.add(ipn));
            }

            MacEntry ment =
                new NewMacEntry(eaddr, sport, pname, vid, ipn, ident);
            assertEquals(expected, ment.getIpNetworkSet());
        }
    }

    /**
     * Test case for {@link NewMacEntry#getMapPath()}.
     */
    @Test
    public void testGetMapPath() {
        EtherAddress eaddr = new EtherAddress(0x001122334455L);
        SalPort sport = new SalPort(1L, 2L);
        String pname = "port-2";
        int vid = 0;
        IpNetwork ip = null;

        List<VNodeIdentifier<?>> identifiers = new ArrayList<>();
        Collections.addAll(
            identifiers,
            new MacMapIdentifier(new VnodeName("vtn_1"),
                                 new VnodeName("vbr_1")),
            new VlanMapIdentifier(new VnodeName("vtn_2"),
                                  new VnodeName("vbr_2"), "ANY.0"),
            new VBridgeIfIdentifier(new VnodeName("vtn_3"),
                                    new VnodeName("vbr_3"),
                                    new VnodeName("if_3")));

        for (VNodeIdentifier<?> ident: identifiers) {
            MacEntry ment =
                new NewMacEntry(eaddr, sport, pname, vid, ip, ident);
            assertEquals(ident.toString(), ment.getMapPath());
        }
    }
}
