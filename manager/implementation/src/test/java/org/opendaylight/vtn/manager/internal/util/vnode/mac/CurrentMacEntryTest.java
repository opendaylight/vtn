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
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntryBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpAddress;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Address;
import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * JUnit test for {@link CurrentMacEntryTest}.
 */
public class CurrentMacEntryTest extends TestBase {
    /**
     * Test case for {@link MacEntry#getEtherAddress()}.
     */
    @Test
    public void testGetEtherAddress() {
        SalPort sport = new SalPort(1L, 2L);
        String pname = "port-2";
        int vid = 0;
        VNodeIdentifier<?> ident = new MacMapIdentifier(
            new VnodeName("vtn_1"), new VnodeName("vbr_1"));

        EtherAddress[] eaddrs = {
            new EtherAddress(0x001122334455L),
            new EtherAddress(0x000000000001L),
            new EtherAddress(0xfafbfcfdfeffL),
        };

        for (EtherAddress eaddr: eaddrs) {
            MacTableEntry mtent = new MacTableEntryBuilder().
                setMacAddress(eaddr.getMacAddress()).
                setNode(sport.getNodeId()).
                setPortId(String.valueOf(sport.getPortNumber())).
                setPortName(pname).
                setVlanId(vid).
                setEntryData(ident.toString()).
                setIpProbeCount(0).
                build();
            MacEntry ment = new CurrentMacEntry(eaddr, mtent);
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
            MacTableEntry mtent = new MacTableEntryBuilder().
                setMacAddress(eaddr.getMacAddress()).
                setNode(sport.getNodeId()).
                setPortId(String.valueOf(sport.getPortNumber())).
                setPortName(pname).
                setVlanId(vid).
                setEntryData(ident.toString()).
                setIpProbeCount(0).
                build();
            MacEntry ment = new CurrentMacEntry(eaddr, mtent);
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
        VNodeIdentifier<?> ident = new MacMapIdentifier(
            new VnodeName("vtn_1"), new VnodeName("vbr_1"));

        int[] vids = {0, 1, 123, 1234, 4000, 4095};

        for (int vid: vids) {
            MacTableEntry mtent = new MacTableEntryBuilder().
                setMacAddress(eaddr.getMacAddress()).
                setNode(sport.getNodeId()).
                setPortId(String.valueOf(sport.getPortNumber())).
                setPortName(pname).
                setVlanId(vid).
                setEntryData(ident.toString()).
                setIpProbeCount(0).
                build();
            MacEntry ment = new CurrentMacEntry(eaddr, mtent);
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
        VnodeName vtnName = new VnodeName("vtn_1");
        VnodeName vbrName = new VnodeName("vbr_1");
        VNodeIdentifier<?> targetIdent =
            new MacMapIdentifier(vtnName, vbrName);
        MacTableEntry mtent = new MacTableEntryBuilder().
            setMacAddress(eaddr.getMacAddress()).
            setNode(targetPort.getNodeId()).
            setPortId(String.valueOf(targetPort.getPortNumber())).
            setPortName(pname).
            setVlanId(targetVid).
            setEntryData(targetIdent.toString()).
            setIpProbeCount(0).
            build();
        MacEntry ment = new CurrentMacEntry(eaddr, mtent);

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
        List<IpAddress> ipaddrs = new ArrayList<>();
        ipaddrs.add(new IpAddress(new Ipv4Address("192.168.100.200")));
        ipaddrs.add(new IpAddress(new Ipv4Address("10.20.30.40")));
        int probe = 3;
        MacTableEntry mtent = new MacTableEntryBuilder().
            setMacAddress(eaddr.getMacAddress()).
            setNode(sport.getNodeId()).
            setPortId(String.valueOf(sport.getPortNumber())).
            setPortName(pname).
            setVlanId(vid).
            setEntryData(ident.toString()).
            setIpAddresses(ipaddrs).
            setIpProbeCount(probe).
            build();
        MacEntry ment = new CurrentMacEntry(eaddr, mtent);
        assertEquals(false, ment.needIpProbe());
        Integer cnt = getFieldValue(ment, MacEntry.class, Integer.class,
                                    "ipProbe");
        assertEquals(probe, cnt.intValue());

        // In case where no IP address is specified.
        mtent = new MacTableEntryBuilder().
            setMacAddress(eaddr.getMacAddress()).
            setNode(sport.getNodeId()).
            setPortId(String.valueOf(sport.getPortNumber())).
            setPortName(pname).
            setVlanId(vid).
            setEntryData(ident.toString()).
            setIpProbeCount(0).
            build();
        ment = new CurrentMacEntry(eaddr, mtent);
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

        // In case where IP address is specified.
        List<IpAddress> ipaddrs = new ArrayList<>();
        Ip4Network ipn = new Ip4Network("192.168.100.200");
        ipaddrs.add(ipn.getIpAddress());
        ipaddrs.add(new IpAddress(new Ipv4Address("10.20.30.40")));
        Set<IpAddress> addrSet = new HashSet<>(ipaddrs);
        int probe = 5;
        MacTableEntry mtent = new MacTableEntryBuilder().
            setMacAddress(eaddr.getMacAddress()).
            setNode(nid).
            setPortId(String.valueOf(pnum)).
            setPortName(pname).
            setVlanId(vid).
            setEntryData(ident.toString()).
            setIpAddresses(ipaddrs).
            setIpProbeCount(probe).
            build();
        MacEntry ment = new CurrentMacEntry(eaddr, mtent);
        assertEquals(null, ment.getNewEntry());
        ment.addIpAddress(ipn);
        assertEquals(null, ment.getNewEntry());

        ipn = new Ip4Network("1.2.3.4");
        ment.addIpAddress(ipn);
        mtent = ment.getNewEntry();

        ipaddrs.add(ipn.getIpAddress());
        addrSet.add(ipn.getIpAddress());
        assertEquals(mac, mtent.getMacAddress());
        assertEquals(nid, mtent.getNode());
        assertEquals(String.valueOf(pnum), mtent.getPortId());
        assertEquals(pname, mtent.getPortName());
        assertEquals(vid, mtent.getVlanId().intValue());
        assertEquals(ident, VNodeIdentifier.create(mtent.getEntryData()));
        assertEquals(probe, mtent.getIpProbeCount().intValue());
        assertEquals(addrSet, new HashSet<>(mtent.getIpAddresses()));

        // In case where no IP address is specified.
        int probes = 0;
        for (int i = 0; i <= 20; i++) {
            mtent = new MacTableEntryBuilder().
                setMacAddress(eaddr.getMacAddress()).
                setNode(nid).
                setPortId(String.valueOf(pnum)).
                setPortName(pname).
                setVlanId(vid).
                setEntryData(ident.toString()).
                setIpProbeCount(probes).
                build();
            ment = new CurrentMacEntry(eaddr, mtent);

            if (i < 10) {
                assertEquals(true, ment.needIpProbe());
                probes = i + 1;
                mtent = ment.getNewEntry();
                assertEquals(mac, mtent.getMacAddress());
                assertEquals(nid, mtent.getNode());
                assertEquals(String.valueOf(pnum), mtent.getPortId());
                assertEquals(pname, mtent.getPortName());
                assertEquals(vid, mtent.getVlanId().intValue());
                assertEquals(ident,
                             VNodeIdentifier.create(mtent.getEntryData()));
                assertEquals(probes, mtent.getIpProbeCount().intValue());
                assertEquals(null, mtent.getIpAddresses());
            } else {
                assertEquals(false, ment.needIpProbe());
                probes = 10;
                assertEquals(null, ment.getNewEntry());
            }
        }

        ipn = new Ip4Network("23.45.67.89");
        ment.addIpAddress(ipn);
        ipaddrs.clear();
        ipaddrs.add(ipn.getIpAddress());
        mtent = ment.getNewEntry();
        assertEquals(mac, mtent.getMacAddress());
        assertEquals(nid, mtent.getNode());
        assertEquals(String.valueOf(pnum), mtent.getPortId());
        assertEquals(pname, mtent.getPortName());
        assertEquals(vid, mtent.getVlanId().intValue());
        assertEquals(ident, VNodeIdentifier.create(mtent.getEntryData()));
        assertEquals(10, mtent.getIpProbeCount().intValue());
        assertEquals(ipaddrs, mtent.getIpAddresses());
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
        int[] vids = {0, 123, 4095};

        List<List<IpAddress>> ipLists = new ArrayList<>();
        ipLists.add(null);

        List<IpAddress> ipaddrs = new ArrayList<>();
        ipaddrs.add(new IpAddress(new Ipv4Address("192.168.1.2")));
        ipLists.add(ipaddrs);

        ipaddrs = new ArrayList<>();
        ipaddrs.add(new IpAddress(new Ipv4Address("192.168.10.20")));
        ipaddrs.add(new IpAddress(new Ipv4Address("10.20.30.40")));
        ipLists.add(ipaddrs);

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
                NodeId nid = sport.getNodeId();
                String pnum = String.valueOf(sport.getPortNumber());
                for (int vid: vids) {
                    for (List<IpAddress> ips: ipLists) {
                        String ipStr;
                        if (ips == null) {
                            ipStr = "";
                        } else {
                            Set<IpNetwork> ipSet = new HashSet<>();
                            for (IpAddress ip: ips) {
                                ipSet.add(IpNetwork.create(ip));
                            }
                            ipStr = ", ipaddr=" + ipSet;
                        }
                        for (VNodeIdentifier<?> ident: identifiers) {
                            MacTableEntry mtent = new MacTableEntryBuilder().
                                setMacAddress(eaddr.getMacAddress()).
                                setNode(nid).
                                setPortId(pnum).
                                setPortName(pname).
                                setVlanId(vid).
                                setEntryData(ident.toString()).
                                setIpAddresses(ips).
                                setIpProbeCount(0).
                                build();
                            MacEntry ment =
                                new CurrentMacEntry(eaddr, mtent);
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
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link CurrentMacEntry#addIpAddress(IpNetwork)}</li>
     *   <li>{@link CurrentMacEntry#getIpAddresses()}</li>
     *   <li>{@link CurrentMacEntry#getIpNetworkSet()}</li>
     * </ul>
     */
    @Test
    public void testIpAddress() {
        EtherAddress eaddr = new EtherAddress(0x001122334455L);
        SalPort sport = new SalPort(1L, 2L);
        String pname = "port-2";
        int vid = 0;
        VNodeIdentifier<?> ident = new MacMapIdentifier(
            new VnodeName("vtn_1"), new VnodeName("vbr_1"));

        MacTableEntry mtent = new MacTableEntryBuilder().
            setMacAddress(eaddr.getMacAddress()).
            setNode(sport.getNodeId()).
            setPortId(String.valueOf(sport.getPortNumber())).
            setPortName(pname).
            setVlanId(vid).
            setEntryData(ident.toString()).
            setIpProbeCount(0).
            build();
        MacEntry ment = new CurrentMacEntry(eaddr, mtent);

        List<IpAddress> ipaddrs = null;
        Set<IpNetwork> ipSet = new HashSet<>();
        assertEquals(ipaddrs, ment.getIpAddresses());
        assertEquals(ipSet, ment.getIpNetworkSet());

        IpNetwork[] addrs = {
            new Ip4Network("10.20.30.40"),
            new Ip4Network("192.168.33.241"),
            new Ip4Network("1.2.3.4"),
        };

        ipaddrs = new ArrayList<>();
        Set<IpAddress> addrSet = new HashSet<>();
        for (IpNetwork ipn: addrs) {
            IpAddress ip = ipn.getIpAddress();
            ipaddrs.add(ip);
            assertTrue(addrSet.add(ip));
            assertTrue(ipSet.add(ipn));

            for (int i = 0; i < 3; i++) {
                ment.addIpAddress(ipn);
                assertEquals(addrSet, new HashSet<>(ment.getIpAddresses()));
                assertEquals(ipSet, ment.getIpNetworkSet());
            }
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
            MacTableEntry mtent = new MacTableEntryBuilder().
                setMacAddress(eaddr.getMacAddress()).
                setNode(sport.getNodeId()).
                setPortId(String.valueOf(sport.getPortNumber())).
                setPortName(pname).
                setVlanId(vid).
                setEntryData(ident.toString()).
                setIpProbeCount(0).
                build();
            MacEntry ment = new CurrentMacEntry(eaddr, mtent);
            assertEquals(ident.toString(), ment.getMapPath());
        }
    }
}
