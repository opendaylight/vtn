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
import java.util.Arrays;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.BeforeClass;
import org.junit.Test;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.switchmanager.SubnetConfig;
import org.opendaylight.controller.topologymanager.ITopologyManager;
import org.opendaylight.vtn.manager.VBridgePath;


public class ArpHandlerTest extends VTNManagerImplTestCommon {

    private byte[] defaultIp = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
    private byte[] gwIp = new byte[] {(byte)192, (byte)168, (byte)0, (byte)254};
    private byte[] hostIp = new byte[] {(byte)192, (byte)168, (byte)0, (byte)251};
    private byte[] hostmac = new byte [] { 0x00, 0x00, 0x00, 0x11, 0x22, 0x33};

    @BeforeClass
    public static void beforeClass() {
        stubMode = 2;
    }

    /**
     * test case for {@link ArpHander#receive}
     */
    @Test
    public void testReceive() {
        VTNManagerImpl mgr = vtnMgr;
        TestStub stub = stubObj;
        ISwitchManager swmgr = mgr.getSwitchManager();
        ITopologyManager topoMgr = mgr.getTopologyManager();
        byte [] src = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                  (byte)0x00, (byte)0x00, (byte)0x11};
        byte [] srcmc = new byte[] {(byte)0x01, (byte)0x00, (byte)0x00,
                                    (byte)0x00, (byte)0x00, (byte)0x11};
        byte [] dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                  (byte)0xff, (byte)0xff, (byte)0xff};
        byte [] sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)1};
        byte [] target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)250};
        byte [] targetht = new byte[] {(byte)192, (byte)168, (byte)0, (byte)251};

        Set<Node> existNodes = swmgr.getNodes();
        Set<NodeConnector> existConnectors = new HashSet<NodeConnector>();
        for (Node node: existNodes) {
            existConnectors.addAll(swmgr.getNodeConnectors(node));
        }

        Set<Node> testNodes = new HashSet<Node>(existNodes);
        testNodes.add(null);

        for (Node node: testNodes) {
            int numMapped = 0;
            Set<NodeConnector> mappedThis = new HashSet<NodeConnector>();
            Set<NodeConnector> noMappedThis = new HashSet<NodeConnector>(existConnectors);

            List<String> ncstr = new ArrayList<String>();

            for (NodeConnector nc : existConnectors) {
                if (node == null || node.equals(nc.getNode())) {
                    if (!topoMgr.isInternal(nc)) {
                        if (node != null) {
                            ncstr.add(nc.toString());
                        }
                        mappedThis.add(nc);
                    }
                }
            }

            SubnetConfig sconf = new SubnetConfig("test", "192.168.0.254/24", ncstr);
            swmgr.addSubnet(sconf);

            noMappedThis.removeAll(mappedThis);
            numMapped = mappedThis.size();

            // null
            PacketResult result = mgr.receiveDataPacket(null);
            assertEquals(PacketResult.IGNORED, result);

            testReceiveDataPacketBCLoop(mgr, null, (short)0,
                    mappedThis, noMappedThis, stub);

            testReceiveDataPacketARPReplyReceive(mgr, null, (short)0,
                    mappedThis, noMappedThis, stub);

            testReceiveDataPacketBCLoopMatch(mgr, null, (short)0,
                    mappedThis, noMappedThis, stub);

            testReceiveDataPacketBCLoopToCont(mgr, null, (short)0,
                    mappedThis, noMappedThis, stub);

            testReceiveDataPacketUCLoop(mgr, null, (short)0,
                    mappedThis, noMappedThis, stub);

            // receive packet form no mapped nodeconector.
            if (node != null && noMappedThis.size() != 0) {
                NodeConnector nc = noMappedThis.iterator().next();
                RawPacket inPkt = createARPRawPacket(src, dst, sender, target,
                        (short)0, nc, ARP.REPLY);
                result = mgr.receiveDataPacket(inPkt);
                List<RawPacket> transDatas = stub.getTransmittedDataPacket();
                assertEquals(PacketResult.IGNORED, result);
                assertEquals(0, transDatas.size());
            }

            swmgr.removeSubnet(sconf);
        }

        // invalid data received case
        Set<NodeConnector> mappedThis = new HashSet<NodeConnector>();
        Set<NodeConnector> noMappedThis = new HashSet<NodeConnector>(existConnectors);
        Set<String> ncstr = new HashSet<String>();

        for (NodeConnector nc : existConnectors) {
            if (!topoMgr.isInternal(nc)) {
                    mappedThis.add(nc);
            }
        }

        // payload is !ARP && !IPv4
        NodeConnector nc = existConnectors.iterator().next();
        RawPacket inPkt = createARPRawPacket(src, dst, sender, target, (short)0,
                          nc, ARP.REQUEST);
        Ethernet pkt= (Ethernet)stub.decodeDataPacket(inPkt);
        pkt.setPayload(null);
        pkt.setEtherType((short)0);
        inPkt = stub.encodeDataPacket(pkt);
        PacketResult result = mgr.receiveDataPacket(inPkt);
        List<RawPacket> transDatas = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.IGNORED, result);
        assertEquals(0, transDatas.size());

        // !ARP.REQUEST && !ARP.REPLY
        inPkt = createARPRawPacket(src, dst, sender, target, (short)0,
                                            nc, ARP.PROTO_TYPE_IP);
        result = mgr.receiveDataPacket(inPkt);
        transDatas = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.IGNORED, result);
        assertEquals(0, transDatas.size());

        // subnet == null
        inPkt = createARPRawPacket(src, dst, sender, target, (short)0,
                                    nc, ARP.REPLY);
        result = mgr.receiveDataPacket(inPkt);
        transDatas = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.IGNORED, result);
        assertEquals(0, transDatas.size());

        inPkt = createIPv4RawPacket(src, dst, sender, target, (short)0,
                nc);
        result = mgr.receiveDataPacket(inPkt);
        transDatas = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.IGNORED, result);
        assertEquals(0, transDatas.size());

        SubnetConfig sconf = new SubnetConfig("test", "192.168.0.254/24", null);
        swmgr.addSubnet(sconf);

        // src is not unicast packet
        inPkt = createARPRawPacket(srcmc, dst, sender, target, (short)0,
                nc, ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);
        transDatas = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        assertEquals(mappedThis.size(), transDatas.size());

        // sender == target
        inPkt = createARPRawPacket(srcmc, dst, sender, sender, (short)0,
                nc, ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);
        transDatas = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.IGNORED, result);
        assertEquals(0, transDatas.size());

        // target IP == controller and dst == unicast
        inPkt = createARPRawPacket(src, src,
                sender, gwIp, (short)0, nc, ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);
        transDatas = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        assertEquals(mappedThis.size(), transDatas.size());

        // !isBroadCast(dst) && host.dladdr != dst
        inPkt = createARPRawPacket(src, swmgr.getControllerMAC(),
                sender, gwIp, (short)0, nc, ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);
        transDatas = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        assertEquals(1, transDatas.size());

        // host found but dst is not match
        inPkt = createARPRawPacket(src, src,
                sender, targetht, (short)0, nc, ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);
        transDatas = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.IGNORED, result);
        assertEquals(0, transDatas.size());

        // host found and dst is not BC
        // host found but dst is not match
        inPkt = createARPRawPacket(src, new byte [] { 0x00, 0x00, 0x00, 0x11, 0x22, 0x33},
                sender, targetht, (short)0, nc, ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);
        transDatas = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        assertEquals(1, transDatas.size());

    }

    /**
     * test case for {@link ArpHander#probe} and {@link ArpHander#find}
     */
    @Test
    public void testFindProbe() {
        VTNManagerImpl mgr = vtnMgr;
        TestStub stub = stubObj;
        ISwitchManager swmgr = mgr.getSwitchManager();
        ITopologyManager topoMgr = mgr.getTopologyManager();

        Set<Node> existNodes = swmgr.getNodes();
        Set<NodeConnector> existConnectors = new HashSet<NodeConnector>();
        for (Node node: existNodes) {
            existConnectors.addAll(swmgr.getNodeConnectors(node));
        }

        Set<Node> testNodes = new HashSet<Node>(existNodes);
        testNodes.add(null);

        InetAddress ia6 = null;
        try {
            ia6 = InetAddress.getByAddress(new byte[]{
                    (byte)0x20, (byte)0x01, (byte)0x04, (byte)0x20,
                    (byte)0x02, (byte)0x81, (byte)0x10, (byte)0x04,
                    (byte)0x0e1, (byte)0x23, (byte)0xe6, (byte)0x88,
                    (byte)0xd6, (byte)0x55, (byte)0xa1, (byte)0xb0});
        } catch (UnknownHostException e) {
            unexpected(e);
        }

        InetAddress ia = null;
        try {
            ia = InetAddress.getByAddress(new byte[] {
                    (byte)10, (byte)0, (byte)0, (byte)1});
        } catch (UnknownHostException e) {
            unexpected(e);
        }

        byte [] mac = new byte [] { 0x00, 0x00, 0x00, 0x11, 0x22, 0x33};
        for (Node node: testNodes) {
            int numMapped = 0;
            Set<NodeConnector> mappedThis = new HashSet<NodeConnector>();
            Set<NodeConnector> noMappedThis = new HashSet<NodeConnector>(existConnectors);

            List<String> ncstr = new ArrayList<String>();
            for (NodeConnector nc : existConnectors) {
                if (node == null || node.equals(nc.getNode())) {
                    if (!topoMgr.isInternal(nc)) {
                        if (node != null) {
                            ncstr.add(nc.toString());
                        }
                        mappedThis.add(nc);
                    }
                }
            }
            SubnetConfig sconf = new SubnetConfig("test", "192.168.0.254/24", ncstr);
            swmgr.addSubnet(sconf);

            noMappedThis.removeAll(mappedThis);
            numMapped = mappedThis.size();

            mgr.find(ia);

            List<RawPacket> transDatas = stub.getTransmittedDataPacket();
            assertEquals(numMapped, transDatas.size());

            for (RawPacket raw : transDatas) {
                Packet pkt = stub.decodeDataPacket(raw);
                assertTrue(raw.getOutgoingNodeConnector().toString(),
                        mappedThis.contains(raw.getOutgoingNodeConnector()));
                assertFalse(raw.getOutgoingNodeConnector().toString(),
                        noMappedThis.contains(raw.getOutgoingNodeConnector()));

                Ethernet eth = (Ethernet)pkt;

                checkOutEthernetPacket("", eth, EtherTypes.ARP,
                    swmgr.getControllerMAC(), new byte[] {-1, -1, -1, -1, -1, -1},
                    (short)0, EtherTypes.IPv4, ARP.REQUEST,
                    swmgr.getControllerMAC(), new byte[] {0, 0, 0, 0, 0, 0},
                    gwIp, ia.getAddress());
            }

            mgr.find(ia6);
            transDatas = stub.getTransmittedDataPacket();
            assertEquals(0, transDatas.size());

            // probe
            Node hnode = NodeCreator.createOFNode(0L);
            NodeConnector nc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"), hnode);
            HostNodeConnector host = null;
            try {
                host = new HostNodeConnector(mac, ia, nc, (short)0);
            } catch (ConstructionException e) {
                unexpected(e);
            }

            mgr.probe(host);

            transDatas = stub.getTransmittedDataPacket();
            assertEquals(1, transDatas.size());

            RawPacket raw = transDatas.get(0);
            assertEquals(nc, raw.getOutgoingNodeConnector());

            Ethernet eth = (Ethernet)stub.decodeDataPacket(raw);
            checkOutEthernetPacket("", eth, EtherTypes.ARP,
                    swmgr.getControllerMAC(), mac,
                    (short)0, EtherTypes.IPv4, ARP.REQUEST,
                    swmgr.getControllerMAC(), mac,
                    gwIp, ia.getAddress());

            swmgr.removeSubnet(sconf);

            // subnet == null
            mgr.find(ia);
            transDatas = stub.getTransmittedDataPacket();
            assertEquals(0, transDatas.size());

            mgr.probe(host);
            transDatas = stub.getTransmittedDataPacket();
            assertEquals(0, transDatas.size());
        }


        // if null
        mgr.probe(null);
    }


    // private methods

    private void testReceiveDataPacketBCLoop(VTNManagerImpl mgr, VBridgePath bpath,
            short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub) {
         testReceiveDataPacketAHCommonLoop(mgr, bpath, vlan,
                 mappedThis, noMappedThis, stub, (short)1, EtherTypes.ARP, ARP.REQUEST, null, defaultIp);
    }

    private void testReceiveDataPacketUCLoop(VTNManagerImpl mgr, VBridgePath bpath,
            short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub) {
        testReceiveDataPacketAHCommonLoop(mgr, bpath, vlan,
                mappedThis, noMappedThis, stub, (short)0, EtherTypes.IPv4, (short)-1, null, defaultIp);
    }

    private void testReceiveDataPacketBCLoopMatch(VTNManagerImpl mgr, VBridgePath bpath,
            short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub) {
         testReceiveDataPacketAHCommonLoop(mgr, bpath, vlan,
                 mappedThis, noMappedThis, stub, (short)1, EtherTypes.ARP, ARP.REQUEST, null, hostIp);
     }

    private void testReceiveDataPacketBCLoopToCont(VTNManagerImpl mgr, VBridgePath bpath,
            short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub) {
         testReceiveDataPacketAHCommonLoop(mgr, bpath, vlan,
                 mappedThis, noMappedThis, stub, (short)1, EtherTypes.ARP, ARP.REQUEST, null, gwIp);
     }

    private void testReceiveDataPacketARPReplyReceive(VTNManagerImpl mgr, VBridgePath bpath,
            short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub) {
         testReceiveDataPacketAHCommonLoop(mgr, bpath, vlan,
                 mappedThis, noMappedThis, stub, (short)1, EtherTypes.ARP, ARP.REPLY, null, defaultIp);
     }

    /**
     * common method for ReceiveDataPacket test using ARPHandler.
     */
    private void testReceiveDataPacketAHCommonLoop(VTNManagerImpl mgr, VBridgePath bpath,
            short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub, short isBc, EtherTypes ethType, short arpType,
            NodeConnector targetnc, byte [] targetIp) {

        ISwitchManager swmgr = mgr.getSwitchManager();
        byte[] target = targetIp;
        Node hostnode = NodeCreator.createOFNode(Long.valueOf(0));
        NodeConnector hostnc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"), hostnode);
        List<EthernetAddress> ethers = createEthernetAddresses(false);
        boolean isFirst = true;

        for (NodeConnector nc: mappedThis) {
            if (targetnc != null && !targetnc.equals(nc)) {
                continue;
            }
            byte iphost = 1;
            for (EthernetAddress ea: ethers) {
                byte[] bytes = ea.getValue();
                byte [] src;
                byte [] dst;
                byte [] sender;
                if (arpType == ARP.REPLY) {
                    dst = new byte[] {bytes[0], bytes[1], bytes[2],
                            bytes[3], bytes[4], bytes[5]};
                    src = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                            (byte)0xff, (byte)0xff, (byte)0x11};
                    target = new byte[] {(byte)192, (byte)168, (byte)0, (byte)iphost};
                    sender = targetIp;
                } else {
                    src = new byte[] {bytes[0], bytes[1], bytes[2],
                            bytes[3], bytes[4], bytes[5]};
                    if (isBc > 0) {
                        dst = new byte[] {(byte)0xff, (byte)0xff, (byte)0xff,
                                (byte)0xff, (byte)0xff, (byte)0xff};
                    } else {
                        dst = new byte[] {(byte)0x00, (byte)0x00, (byte)0x00,
                                (byte)0xff, (byte)0xff, (byte)0x11};
                    }
                    sender = new byte[] {(byte)192, (byte)168, (byte)0, (byte)iphost};
                }

                RawPacket inPkt = null;
                if (ethType.shortValue() == EtherTypes.IPv4.shortValue()) {
                    inPkt = createIPv4RawPacket(src, dst, sender, target,
                            (vlan > 0) ? vlan : -1, nc);
                } else if (ethType.shortValue() == EtherTypes.ARP.shortValue()){
                    inPkt = createARPRawPacket(src, dst, sender, target,
                            (vlan > 0) ? vlan : -1, nc, arpType);
                }
                Ethernet inPktDecoded = (Ethernet)stub.decodeDataPacket(inPkt);
                PacketResult result = mgr.receiveDataPacket(inPkt);

                if (nc != null &&
                        nc.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                    if (isFirst) {
                        assertEquals(nc.toString() + ea.toString(), PacketResult.KEEP_PROCESSING, result);
                    } else {
                        assertEquals(nc.toString() + ea.toString(), PacketResult.IGNORED, result);
                    }

                    Packet payload = inPktDecoded.getPayload();
                    if (inPktDecoded.getEtherType() == EtherTypes.VLANTAGGED.shortValue()) {
                        payload = payload.getPayload();
                    }

                    List<RawPacket> transDatas = stub.getTransmittedDataPacket();
                    if ((Arrays.equals(target, hostIp) || Arrays.equals(target, gwIp)) &&
                            arpType != ARP.REPLY) {
                        assertEquals(nc.toString() + "," + ea.toString(), 1, transDatas.size());
                    } else if (ethType.shortValue() == EtherTypes.ARP.shortValue() &&
                            arpType == ARP.REPLY) {
                        if (isFirst) {
                            assertEquals(nc.toString() + "," + ea.toString(),
                                mappedThis.size() * ethers.size(), transDatas.size());
                            isFirst = false;
                        } else {
                            assertEquals(nc.toString() + "," + ea.toString(),
                                    0, transDatas.size());
                        }
                    } else {
                        assertEquals(nc.toString() + "," + ea.toString(),
                                mappedThis.size(), transDatas.size());
                    }

                    for (RawPacket raw: transDatas) {
                        Packet pkt = stub.decodeDataPacket(raw);
                        assertTrue(nc.toString() + ea.toString() + raw.getOutgoingNodeConnector(),
                                mappedThis.contains(raw.getOutgoingNodeConnector()));
                        assertFalse(nc.toString() + ea.toString() + raw.getOutgoingNodeConnector(),
                                noMappedThis.contains(raw.getOutgoingNodeConnector()));

                        if (Arrays.equals(target, hostIp) || Arrays.equals(target, gwIp)) {
                            assertEquals(nc.toString() + ea.toString() + raw.getOutgoingNodeConnector(),
                                    nc, raw.getOutgoingNodeConnector());
                        }

                        if (ethType.shortValue() == EtherTypes.IPv4.shortValue()) {
                            // send ARP Request
                            checkOutEthernetPacket(nc.toString() + "," + ea.toString(),
                                    (Ethernet)pkt, EtherTypes.ARP,
                                    swmgr.getControllerMAC(), new byte[] {-1, -1, -1, -1, -1, -1}, vlan,
                                    EtherTypes.IPv4, ARP.REQUEST,
                                    swmgr.getControllerMAC(),
                                    new byte[] {0, 0, 0, 0, 0, 0},
                                    gwIp, target);
                        } else if (ethType.shortValue() == EtherTypes.ARP.shortValue() &&
                                arpType == ARP.REQUEST && Arrays.equals(target, hostIp) ){
                            // in this case receive ARP Reply
                            checkOutEthernetPacket(nc.toString() + "," + ea.toString(),
                                    (Ethernet)pkt, EtherTypes.ARP,
                                    hostmac, src, vlan,
                                    EtherTypes.IPv4, ARP.REPLY,
                                    hostmac, src, target, sender);
                        } else if (ethType.shortValue() == EtherTypes.ARP.shortValue() &&
                                arpType == ARP.REQUEST && Arrays.equals(target, gwIp)){
                            // in this case receive ARP Reply
                            checkOutEthernetPacket(nc.toString() + "," + ea.toString(),
                                    (Ethernet)pkt, EtherTypes.ARP,
                                    swmgr.getControllerMAC(),
                                    src, vlan,
                                    EtherTypes.IPv4, ARP.REPLY,
                                    swmgr.getControllerMAC(),
                                    src,
                                    target, sender);
                        } else if (ethType.shortValue() == EtherTypes.ARP.shortValue() &&
                                    arpType == ARP.REQUEST){
                            // in this case flooded
                            checkOutEthernetPacket(nc.toString() + "," + ea.toString(),
                                    (Ethernet)pkt, EtherTypes.ARP,
                                    swmgr.getControllerMAC(), dst, vlan,
                                    EtherTypes.IPv4, ARP.REQUEST,
                                    swmgr.getControllerMAC(),
                                    new byte[] {0, 0, 0, 0, 0, 0},
                                    gwIp, target);
                        } else if (ethType.shortValue() == EtherTypes.ARP.shortValue() &&
                                    arpType == ARP.REPLY){
                            // in this case flooded
                            checkOutEthernetPacket(nc.toString() + "," + ea.toString(),
                                    (Ethernet)pkt, EtherTypes.ARP,
                                    src, null, vlan,
                                    EtherTypes.IPv4, ARP.REPLY,
                                    src,
                                    null,
                                    sender, null);
                        } else {
                            fail("unexepcted packet received.");
                        }
                    }
                } else {
                    if (nc != null) {
                        assertEquals(nc.toString() + "," + ea.toString(), PacketResult.IGNORED, result);
                    } else {
                        assertEquals(ea.toString(), PacketResult.IGNORED, result);
                    }
                }
                iphost++;
            }
        }
    }
}
