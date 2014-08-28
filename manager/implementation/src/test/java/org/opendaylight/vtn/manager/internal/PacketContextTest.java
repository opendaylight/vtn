/*
 * Copyright (c) 2013-2014 NEC Corporation
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
import java.util.List;
import java.util.Set;
import java.util.concurrent.ConcurrentMap;

import org.junit.Test;

import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.match.Match;
import org.opendaylight.controller.sal.match.MatchField;
import org.opendaylight.controller.sal.match.MatchType;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.PacketException;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

import org.opendaylight.vtn.manager.VBridgeConfig;
import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VInterfaceConfig;
import org.opendaylight.vtn.manager.VNodeRoute.Reason;
import org.opendaylight.vtn.manager.VNodeRoute;
import org.opendaylight.vtn.manager.VTenantConfig;
import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.cluster.FlowGroupId;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;
import org.opendaylight.vtn.manager.internal.cluster.VlanMapPath;

/**
 * JUnit test for {@link PacketContext}.
 */
public class PacketContextTest extends TestUseVTNManagerBase {
    /**
     * Construct a new instance.
     */
    public PacketContextTest() {
        super(2);
    }

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short[] vlans = {-10, 0, 1, 100, 4095};
        Ethernet ether;
        byte[] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                 (byte)0xff, (byte)0xff, (byte)0xff};
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

        for (NodeConnector nc : createNodeConnectors(3, false)) {
            byte iphost = 1;
            for (short vlan : vlans) {
                for (EthernetAddress ea : createEthernetAddresses(false)) {
                    byte[] bytes = ea.getValue();
                    byte[] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                             bytes[3], bytes[4], bytes[5]};
                    byte[] sender = new byte[] {(byte)192, (byte)168,
                                                (byte)0, iphost};
                    byte[] sender2 = new byte[] {(byte)192, (byte)168,
                                                 (byte)0, (byte)(iphost + 100)};

                    testPacketContext(src, dst, sender, target, vlan, nc);

                    ether = createARPPacket(src, dst, sender2, target,
                                            (vlan > 0) ? vlan : -1, ARP.REQUEST);
                    testPacketContext(ether, src, dst, sender2, target, vlan, nc);

                    iphost++;
                }
            }

        }
    }

    /**
     * Test case for {@link PacketContext#PacketContext(RawPacket, Ethernet)}.
     *
     * @param src       src MAC address.
     * @param dst       dst MAC address.
     * @param sender    sender IP address.
     * @param target    target IP address.
     * @param vlan      VLAN ID
     * @param nc        Node Connector.
     */
    private void testPacketContext(byte[] src, byte[] dst, byte[] sender,
                                   byte[] target, short vlan, NodeConnector nc) {
        InetAddress ipaddr = null;
        MacTableEntry me;
        PortVlan pv;
        String desc;
        String msg = createErrorMessageString(src, dst, sender, target, vlan, nc);

        // createARPPacketContext create PacketContext
        // by using {@link PacketContext(RawPacket, Ethernet)}.
        PacketContext pc = createARPPacketContext(src, dst, sender, target,
                                                  (vlan > 0) ? vlan : -1, nc,
                                                  ARP.REQUEST);

        VBridgePath path = new VBridgePath("tenant1", "bridge1");
        Long key = NetUtils.byteArray6ToLong(src);
        // test getter methods
        if (vlan <= 0) {
            assertEquals(msg, 0, pc.getVlan());
            pv = new PortVlan(nc, (short)0);
            me = new MacTableEntry(path, key, nc, (short)0, ipaddr);
        } else {
            assertEquals(msg, vlan, pc.getVlan());
            pv = new PortVlan(nc, vlan);
            me = new MacTableEntry(path, key, nc, vlan, ipaddr);
        }

        assertNotNull(msg, pc.getRawPacket());
        assertNotNull(msg, pc.getIncomingNodeConnector());
        assertEquals(msg, pv, pc.getIncomingNetwork());

        assertNull(msg, pc.getOutgoingNodeConnector());
        assertEquals(msg, src, pc.getSourceAddress());
        assertEquals(msg, dst, pc.getDestinationAddress());

        byte[] sip = pc.getSourceIpAddress();
        assertEquals(msg, sender, sip);

        try {
            ipaddr = InetAddress.getByAddress(sip);
        } catch (UnknownHostException e) {
            // This should never happen.
            fail(e.getMessage());
        }

        pc.addObsoleteEntry(me);

        HashSet<ObjectPair<MacVlan, NodeConnector>> obsoletes =
            new HashSet<ObjectPair<MacVlan, NodeConnector>>();
        obsoletes.add(getL2Host(me));
        assertEquals(msg, obsoletes, pc.getObsoleteEntries());

        // test getFrame()
        Ethernet ether = pc.getFrame();
        checkOutEthernetPacket("", ether, EtherTypes.ARP, src, dst, vlan,
                               EtherTypes.IPv4, ARP.REQUEST, src, dst,
                               sender, target);

        // test createFrame()
        Ethernet newether;
        short ethType = EtherTypes.ARP.shortValue();
        if (vlan <= 0) {
            desc = convertForDescription(ether, ethType, nc, (short)0);
            newether = pc.createFrame((short)0);
        } else {
            desc = convertForDescription(ether, ethType, nc, vlan);
            newether = pc.createFrame(vlan);
        }
        assertEquals(msg, desc, pc.getDescription(nc));
        assertNotNull(msg, newether);
        assertEquals(msg, ether, newether);
        assertNotSame(msg, ether, newether);


        // in case payload is null && RawPayload is null
        PacketContext pctxp = null;
        short newvlan = 0;
        if (vlan <= 0) {
            ether.setPayload(null);
            pctxp = new PacketContext(null, ether);
        } else {
            Packet tag = ether.getPayload();
            tag.setPayload(null);
            pctxp = new PacketContext(null, ether);
            newvlan = vlan;
        }
        desc = convertForDescription(ether, ethType, nc, newvlan);
        newether = pctxp.createFrame(newvlan);
        assertEquals(msg, desc, pctxp.getDescription(nc));
        assertEquals(msg, newether, pctxp.getFrame());
        ARP arp = getPayload(newether, EtherTypes.ARP, (vlan > 0) ? vlan : -1, msg);
        assertNull(msg, arp);
        assertArrayEquals(msg, src, newether.getSourceMACAddress());
        assertArrayEquals(msg, dst, newether.getDestinationMACAddress());
        assertNotNull(msg, newether);

        // in case payload is null && RawPayload is not null
        RawPacket raw = null;
        try {
            raw = new RawPacket(ether.serialize());
            ether.setRawPayload(ether.serialize());
        } catch (ConstructionException e) {
            unexpected(e);
        } catch (PacketException e) {
            unexpected(e);
        }

        newvlan = 0;
        if (vlan <= 0) {
            ether.setPayload(null);
            pctxp = new PacketContext(raw, ether);
            assertEquals(msg, ether, pctxp.getFrame());
        } else {
            Packet tag = ether.getPayload();
            tag.setPayload(null);
            tag.setRawPayload(new byte[]{0});
            pctxp = new PacketContext(raw, ether);
            assertEquals(msg, ether, pctxp.getFrame());
            newvlan = vlan;
        }
        desc = convertForDescription(ether, ethType, nc, newvlan);
        newether = pctxp.createFrame(newvlan);
        assertEquals(msg, desc, pctxp.getDescription(nc));
        arp = getPayload(newether, EtherTypes.ARP, (vlan > 0) ? vlan : -1, msg);
        assertNull(msg, arp);
        assertArrayEquals(msg, src, newether.getSourceMACAddress());
        assertArrayEquals(msg, dst, newether.getDestinationMACAddress());
        assertNotNull(msg, newether);
    }

    /**
     * Test case for {@link PacketContext#PacketContext(RawPacket, Ethernet)}.
     *
     * @param ether     Ethernet frame
     * @param src       src MAC address
     * @param dst       dst MAC address
     * @param sender    sender IP address
     * @param target    target IP address
     * @param vlan      VLAN ID
     * @param nc        a Node Connector.
     */
    private void testPacketContext(Ethernet ether, byte[] src, byte[] dst, byte[] sender, byte[] target, short vlan, NodeConnector nc) {
        InetAddress ipaddr = null;
        MacTableEntry me;
        String desc;
        String msg = createErrorMessageString(src, dst, sender, target, vlan, nc);

        PacketContext pctx = new PacketContext(ether, nc);

        // test getter methods
        assertNull(msg, pctx.getRawPacket());
        assertNull(msg, pctx.getIncomingNetwork());
        try {
            assertNull(msg, pctx.getIncomingNodeConnector());
        } catch (NullPointerException expected) {
            assertEquals(msg, expected.getMessage(), null);
        }

        assertEquals(msg, nc, pctx.getOutgoingNodeConnector());
        assertNotNull(msg, pctx.getPayload());
        assertEquals(msg, src, pctx.getSourceAddress());
        assertEquals(msg, dst, pctx.getDestinationAddress());
        assertEquals(msg, sender, pctx.getSourceIpAddress());

        byte[] sip = pctx.getSourceIpAddress();
        try {
            ipaddr = InetAddress.getByAddress(sip);
        } catch (UnknownHostException e) {
            // This should never happen.
            fail(e.getMessage());
        }

        Long key = NetUtils.byteArray6ToLong(src);
        VBridgePath path = new VBridgePath("tenant1", "bridge1");
        if (vlan < 0) {
            assertEquals(msg, (short)0, pctx.getVlan());
            me = new MacTableEntry(path, key, nc, (short)0, ipaddr);
        } else {
            assertEquals(vlan, pctx.getVlan());
            me = new MacTableEntry(path, key, nc, vlan, ipaddr);
        }

        pctx.addObsoleteEntry(me);

        HashSet<ObjectPair<MacVlan, NodeConnector>> obsoletes =
            new HashSet<ObjectPair<MacVlan, NodeConnector>>();
        obsoletes.add(getL2Host(me));
        assertEquals(msg, obsoletes, pctx.getObsoleteEntries());

        // test getFrame()
        assertEquals(ether, pctx.getFrame());
        checkOutEthernetPacket(msg, ether, EtherTypes.ARP, src, dst, vlan, EtherTypes.IPv4,
                ARP.REQUEST, src, dst, sender, target);

        // test createFrame()
        Ethernet newether;
        short ethType = EtherTypes.ARP.shortValue();
        if (vlan < 0) {
            desc = convertForDescription(ether, ethType, nc, (short)0);
            newether = pctx.createFrame((short)0);
        } else {
            desc = convertForDescription(ether, ethType, nc, vlan);
            newether = pctx.createFrame(vlan);
        }
        assertEquals(msg, desc, pctx.getDescription(nc));
        assertNotNull(msg, newether);
        assertEquals(msg, ether, newether);
        assertNotSame(msg, ether, newether);

        if (vlan <= 0) {
            // set invalid sender ip address
            ARP arp = (ARP)newether.getPayload();
            arp.setProtocolType(EtherTypes.IPv4.shortValue());
            arp.setSenderProtocolAddress(new byte[] {0});
            PacketContext pctxl = new PacketContext(newether, nc);
            assertEquals(msg, null, pctxl.getSourceIpAddress());
            assertEquals(msg, null, pctxl.getSourceIpAddress());

        } else {
            // case of invalid source address
            IEEE8021Q tag = (IEEE8021Q)newether.getPayload();
            ARP arp = (ARP)tag.getPayload();
            arp.setProtocolType(EtherTypes.LLDP.shortValue());
            PacketContext pctxl = new PacketContext(newether, nc);
            assertEquals(msg, null, pctxl.getSourceIpAddress());

            // set invalid sender ip address
            arp.setProtocolType(EtherTypes.IPv4.shortValue());
            arp.setSenderProtocolAddress(new byte[] {0});
            pctxl = new PacketContext(newether, nc);
            assertEquals(msg, null, pctxl.getSourceIpAddress());
            assertEquals(msg, null, pctxl.getSourceIpAddress());
        }

        // in case have VLAN tag (include a case vlan id == 0)
        if (vlan >= 0) {
            ARP arp = new ARP();
            arp.setHardwareType(ARP.HW_TYPE_ETHERNET).
                    setProtocolType(EtherTypes.IPv4.shortValue()).
                    setHardwareAddressLength((byte)EthernetAddress.SIZE).
                    setProtocolAddressLength((byte)target.length).
                    setOpCode(ARP.REQUEST).
                    setSenderHardwareAddress(src).setSenderProtocolAddress(sender).
                    setTargetHardwareAddress(dst).setTargetProtocolAddress(target);

            Ethernet eth = new Ethernet();
            eth.setSourceMACAddress(src).setDestinationMACAddress(dst);

            eth.setEtherType(EtherTypes.VLANTAGGED.shortValue());

            IEEE8021Q vlantag = new IEEE8021Q();
            vlantag.setCfi((byte)0x0).setPcp((byte)0x0).setVid((short)vlan).
                setEtherType(EtherTypes.ARP.shortValue()).setParent(eth);
            vlantag.setPayload(arp);
            eth.setPayload(vlantag);

            pctx = new PacketContext(eth, nc);
            newether = pctx.createFrame(vlan);

            assertNotNull(msg, newether);
            assertEquals(msg, eth, newether);
            assertNotSame(msg, eth, newether);
        }
    }

    /**
     * Test case for {@link PacketContext#probeInetAddress(VTNManagerImpl)}.
     */
    @Test
    public void testProbeInetAddress() {
        byte[] src = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                 (byte)0x00, (byte)0x00, (byte)0x01 };
        byte[] dst = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                 (byte)0x00, (byte)0x00, (byte)0x02 };
        byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};

        short vlan = 1;

        Node node = NodeCreator.createOFNode(0L);
        NodeConnector ncIn
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short)10), node);

        // in case IPv4Packet.
        Ethernet eth = createIPv4Packet(src, dst, sender, target, vlan);
        RawPacket raw = null;
        try {
            raw = new RawPacket(eth.serialize());
        } catch (ConstructionException e) {
            unexpected(e);
        } catch (PacketException e) {
            unexpected(e);
        }
        raw.setIncomingNodeConnector(ncIn);
        PacketContext pctx = new PacketContext(raw, eth);

        pctx.probeInetAddress(vtnMgr);
        List<RawPacket> datas = stubObj.getTransmittedDataPacket();

        assertEquals(1, datas.size());
        raw = datas.get(0);
        Ethernet ethOut = (Ethernet)stubObj.decodeDataPacket(raw);
        checkOutEthernetPacket("", ethOut, EtherTypes.ARP, stubObj.getControllerMAC(), src,
                vlan, EtherTypes.IPv4, ARP.REQUEST,
                stubObj.getControllerMAC(), src, new byte[]{0, 0, 0, 0}, sender);

        // invalid case (not IPv4 packet)
        eth = createARPPacket(src, dst, sender, target, vlan, ARP.REQUEST);
        raw = null;
        try {
            raw = new RawPacket(eth.serialize());
        } catch (ConstructionException e) {
            unexpected(e);
        } catch (PacketException e) {
            unexpected(e);
        }
        raw.setIncomingNodeConnector(ncIn);
        pctx = new PacketContext(raw, eth);

        pctx.probeInetAddress(vtnMgr);
        datas = stubObj.getTransmittedDataPacket();

        assertEquals(0, datas.size());
    }

    /**
     * test case for {@link PacketContext#purgeObsoleteFlow(VTNManagerImpl, String)}.
     */
    @Test
    public void testPurgeObsoleteFlow() {
        // create a tenant and a bridge.
        VTenantPath tpath = new VTenantPath("tenant");
        VBridgePath bpath1 = new VBridgePath(tpath.getTenantName(), "bridge1");
        VBridgePath bpath2 = new VBridgePath(tpath.getTenantName(), "bridge2");
        VBridgeIfPath ipath1 = new VBridgeIfPath(bpath1, "if_1");
        VBridgeIfPath ipath2 = new VBridgeIfPath(bpath2, "if_1");
        VlanMapPath vpath1 = new VlanMapPath(bpath1, "ANY.0");
        VlanMapPath vpath2 = new VlanMapPath(bpath2, "ANY.0");
        VTenantPath[] allPaths = {
            tpath, bpath1, bpath2, ipath1, ipath2, vpath1, vpath2,
        };

        Status st = vtnMgr.addTenant(tpath, new VTenantConfig("desc"));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = vtnMgr.addBridge(bpath1, new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        st = vtnMgr.addInterface(
            ipath1, new VInterfaceConfig(null, Boolean.TRUE));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        st = vtnMgr.addBridge(bpath2, new VBridgeConfig(null));
        assertEquals(StatusCode.SUCCESS, st.getCode());
        st = vtnMgr.addInterface(
            ipath2, new VInterfaceConfig(null, Boolean.TRUE));
        assertEquals(StatusCode.SUCCESS, st.getCode());

        short[] vlans = {0, 1, 100, 4095};
        byte[] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                 (byte)0xff, (byte)0xff, (byte)0xff};
        byte[] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
        byte[] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)0};

        Node node = NodeCreator.createOFNode(Long.valueOf(0L));
        NodeConnector innc = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)1), node);
        NodeConnector outnc = NodeConnectorCreator.
            createOFNodeConnector(Short.valueOf((short)2), node);

        VTNFlowDatabase fdb = vtnMgr.getTenantFlowDB(tpath.getTenantName());
        List<EthernetAddress> ethers = createEthernetAddresses(false);
        byte iphost = 1;
        int pri = 1;

        // add obsolete entries.
        int numEntries = 0;
        for (short vlan : vlans) {
            short outVlan = vlan;
            if (outVlan < 4095) {
                outVlan++;
            }

            for (EthernetAddress ea : ethers) {
                Set<VTenantPath> dependPaths = new HashSet<VTenantPath>();

                String msg = "";
                byte[] bytes = ea.getValue();
                byte[] src = new byte[] {bytes[0], bytes[1], bytes[2],
                                         bytes[3], bytes[4], bytes[5]};
                sender[3] = iphost;

                PacketContext pctx = createARPPacketContext(
                    src, dst, sender, target, (vlan > 0) ? vlan : -1,
                    innc, ARP.REQUEST);

                InetAddress ipaddr = null;
                byte[] sip = pctx.getSourceIpAddress();
                try {
                    ipaddr = InetAddress.getByAddress(sip);
                } catch (UnknownHostException e) {
                    // This should never happen.
                    fail(e.getMessage());
                }

                Long key = NetUtils.byteArray6ToLong(src);
                MacTableEntry me;
                if (vlan < 0) {
                    assertEquals(msg, (short)0, pctx.getVlan());
                    me = new MacTableEntry(ipath1, key, innc, (short)0, ipaddr);
                } else {
                    assertEquals(vlan, pctx.getVlan());
                    me = new MacTableEntry(ipath1, key, innc, vlan, ipaddr);
                }

                pctx.addObsoleteEntry(me);
                Set<ObjectPair<MacVlan, NodeConnector>> obsoletes =
                    pctx.getObsoleteEntries();
                assertTrue(obsoletes.contains(getL2Host(me)));
                assertEquals(1, obsoletes.size());

                VTNFlow flow = fdb.create(vtnMgr);
                Match match = pctx.createMatch(innc);

                // check match object.
                for (MatchType type : match.getMatchesList()) {
                    MatchField field = match.getField(type);
                    if (type == MatchType.DL_SRC) {
                        assertEquals(field.getValue(), src);
                    } else if (type == MatchType.DL_DST) {
                        assertEquals(field.getValue(), dst);
                    } else if (type == MatchType.DL_VLAN) {
                        assertEquals(field.getValue(), (vlan > 0) ? vlan : 0);
                    } else if (type == MatchType.IN_PORT) {
                        assertEquals(field.getValue(), innc);
                    } else {
                        fail("unknown matchfiled was found." + field.toString());
                    }
                }

                ActionList actions = new ActionList(innc.getNode());
                actions.addVlanId(outVlan).addOutput(outnc);
                flow.addFlow(vtnMgr, match, actions, pri);
                assertTrue(flow.getFlowPorts().contains(innc));

                // A flow entry must depend on source and destination L2 hosts.
                assertTrue(flow.dependsOn(new MacVlan(src, vlan)));
                assertTrue(flow.dependsOn(new MacVlan(dst, outVlan)));
                boolean sameVlan = (vlan == outVlan);
                assertEquals(sameVlan,
                             flow.dependsOn(new MacVlan(src, outVlan)));
                assertEquals(sameVlan, flow.dependsOn(new MacVlan(dst, vlan)));

                // Set source node path.
                pctx.fixUp(flow);
                assertEquals(null, flow.getIngressPath());
                assertEquals(null, flow.getEgressPath());
                for (VTenantPath p: allPaths) {
                    assertFalse(flow.dependsOn(p));
                }
                flow.clearVirtualRoute();

                pctx.addVNodeRoute(new VNodeRoute(ipath1, Reason.PORTMAPPED));
                pctx.fixUp(flow);
                assertEquals(ipath1, flow.getIngressPath());
                assertEquals(null, flow.getEgressPath());
                assertTrue(dependPaths.add(ipath1));
                assertTrue(dependPaths.add(bpath1));
                assertTrue(dependPaths.add(tpath));
                for (VTenantPath p: allPaths) {
                    assertEquals(dependPaths.contains(p), flow.dependsOn(p));
                }
                flow.clearVirtualRoute();

                // Set destination node path.
                pctx.setEgressVNodePath(vpath1);
                pctx.fixUp(flow);
                assertTrue(dependPaths.add(vpath1));
                assertEquals(ipath1, flow.getIngressPath());
                assertEquals(vpath1, flow.getEgressPath());
                for (VTenantPath p: allPaths) {
                    assertEquals(dependPaths.contains(p), flow.dependsOn(p));
                }

                // Set depended node paths.
                for (VTenantPath pth: new VTenantPath[]{ipath2, vpath2}) {
                    assertFalse(flow.dependsOn(pth));
                    pctx.addNodePath(pth);
                    flow.clearVirtualRoute();
                    pctx.fixUp(flow);
                    assertEquals(ipath1, flow.getIngressPath());
                    assertEquals(vpath1, flow.getEgressPath());
                    assertTrue(dependPaths.add(pth));
                    for (VTenantPath p: allPaths) {
                        assertEquals(dependPaths.contains(p),
                                     flow.dependsOn(p));
                    }
                }

                // install and purge flow.
                fdb.install(vtnMgr, flow);
                flushFlowTasks();
                numEntries++;
                ConcurrentMap<FlowGroupId, VTNFlow> db = vtnMgr.getFlowDB();
                assertEquals(numEntries, db.size());
                assertEquals(numEntries, stubObj.getFlowEntries().size());

                pctx.purgeObsoleteFlow(vtnMgr, tpath.getTenantName());
                obsoletes = pctx.getObsoleteEntries();
                assertTrue(obsoletes.isEmpty());
                flushFlowTasks();
                assertEquals(numEntries - 1, db.size());
                assertEquals(numEntries - 1, stubObj.getFlowEntries().size());

                fdb.install(vtnMgr, flow);
                flushFlowTasks();
                assertEquals(numEntries, db.size());
                assertEquals(numEntries, stubObj.getFlowEntries().size());

                // specify unmatch tenant name.
                pctx.purgeObsoleteFlow(vtnMgr, "unknown");
                obsoletes = pctx.getObsoleteEntries();
                assertTrue(obsoletes.isEmpty());
                flushFlowTasks();
                assertEquals(numEntries, db.size());
                assertEquals(numEntries, stubObj.getFlowEntries().size());

                iphost++;
            }
        }

        // remove flowEntry by specifing VBridgePath
        fdb.removeFlows(vtnMgr, bpath2);
        flushFlowTasks();
        ConcurrentMap<FlowGroupId, VTNFlow> db = vtnMgr.getFlowDB();
        assertEquals(numEntries, db.size());
        assertEquals(numEntries, stubObj.getFlowEntries().size());

        fdb.removeFlows(vtnMgr, bpath1);
        flushFlowTasks();
        assertEquals(0, db.size());
        assertEquals(0, stubObj.getFlowEntries().size());
    }


    // private methods

    /**
     * Create a string to compare description.
     *
     * @param ether An Ethernet Object
     * @param type  An Ethernet type.
     * @param port  A NodeConnector
     * @param vlan  VLAN ID
     * @return  A brief description of the specified ethernet frame.
     */
    private String convertForDescription(Ethernet ether, short type,
                                         NodeConnector port, short vlan) {
        String srcmac = HexEncode.bytesToHexStringFormat(ether.getSourceMACAddress());
        String dstmac = HexEncode.bytesToHexStringFormat(ether.getDestinationMACAddress());
        int itype = (int)type & 0xffff;

        StringBuilder builder = new StringBuilder("src=");
        builder.append(srcmac).append(", dst=")
               .append(dstmac).append(", port=")
               .append(port).append(", type=0x")
               .append(Integer.toHexString(itype))
               .append(", vlan=").append(vlan);

        return builder.toString();
    }

    /**
     * extract payload form Ethrnet object.
     *
     * @param eth       a Ethenet object
     * @param ethType   Ethenet Types
     * @param vlan      a VLAN ID
     * @param msg       error message
     * @return a ARP object
     */
    private ARP getPayload(Ethernet eth, EtherTypes ethType, short vlan, String msg) {
        ARP arp = null;
        if (vlan > 0) {
            assertEquals(msg, EtherTypes.VLANTAGGED.shortValue(), eth.getEtherType());
            IEEE8021Q vlantag = (IEEE8021Q)eth.getPayload();
            assertEquals(msg, vlan, vlantag.getVid());
            assertEquals(msg, ethType.shortValue(), vlantag.getEtherType());
            if (ethType.shortValue() == EtherTypes.ARP.shortValue()) {
                arp = (ARP)vlantag.getPayload();
            }
        } else {
            assertEquals(msg, ethType.shortValue(), eth.getEtherType());
            if (ethType.shortValue() == EtherTypes.ARP.shortValue()) {
                arp = (ARP)eth.getPayload();
            }
        }
        return arp;
    }

    /**
     * create String of Error Message.
     */
    private String createErrorMessageString(byte[] src, byte[] dst, byte[] sender, byte[] target,
            short vlan, NodeConnector nc) {
        StringBuilder builder = new StringBuilder();

        if (src != null) {
            builder.append("(src)");
            for (byte val : src) {
                builder.append(Integer.toHexString(val & 0xff)).append(":");
            }
            builder.replace(builder.length() - 1, builder.length() - 1, ",");
        }

        if (dst != null) {
            builder.append("(dst)");
            for (byte val : dst) {
                builder.append(Integer.toHexString(val & 0xff)).append(":");
            }
            builder.replace(builder.length() - 1, builder.length() - 1, ",");
        }

        if (sender != null) {
            builder.append("(sender)");
            for (byte val : sender) {
                builder.append(Integer.valueOf(val & 0xff).toString()).append(".");
            }
            builder.replace(builder.length() - 1, builder.length() - 1, ",");
        }

        if (target != null) {
            builder.append("(target)");
            for (byte val : target) {
                builder.append(Integer.valueOf(val & 0xff).toString()).append(".");
            }
            builder.replace(builder.length() - 1, builder.length() - 1, ",");
        }

        builder.append("(vlan)").append(vlan).append(",");

        if (nc != null) {
            builder.append("(NodeConnector)").append(nc.toString()).append(",");
        }

        builder.delete(builder.length() - 1, builder.length() - 1);

        return builder.toString();
    }

    /**
     * Create a L2 host entry specified by the given MAC address table entry.
     *
     * @param tent  A MAC address table entry.
     * @return      A pair of {@link MacVlan} object and a node connector
     *              associated with a switch port.
     */
    private ObjectPair<MacVlan, NodeConnector> getL2Host(MacTableEntry tent) {
        MacVlan mv = new MacVlan(tent.getMacAddress(), tent.getVlan());
        return new ObjectPair<MacVlan, NodeConnector>(mv, tent.getPort());
    }
}
