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
import java.util.Arrays;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.junit.Test;
import org.junit.experimental.categories.Category;

import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.LLDP;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.switchmanager.Subnet;
import org.opendaylight.controller.switchmanager.SubnetConfig;
import org.opendaylight.controller.topologymanager.ITopologyManager;
import org.opendaylight.vtn.manager.VBridgePath;

/**
 * JUnit test for {@link ArpHandler}
 */
@Category(SlowTest.class)
public class ArpHandlerTest extends VTNManagerImplTestCommon {

    /**
     * target IP address used test.
     */
    private byte[] testTgtIp = new byte[] {(byte) 192, (byte) 168, (byte) 0, (byte) 250};

    /**
     * gateway IP address.
     */
    private byte[] gwIp = new byte[] {(byte) 192, (byte) 168, (byte) 0, (byte) 254};

    /**
     * target host IP address.
     * {@link TestStub} maintains a {@link HostNodeConnector} information of
     * this IP address Host.
     */
    private byte[] hostIp = new byte[] {(byte) 192, (byte) 168, (byte) 0, (byte) 251};

    /**
     * target host  MAC address.
     * When query {@link HostNodeConnector} of {@code hostIp},
     * {@link TestStub} return this MAC address.
     */
    private byte[] hostMac = new byte [] { 0x00, 0x00, 0x00, 0x11, 0x22, 0x33};

    /**
     * Construct a new instance.
     */
    public ArpHandlerTest() {
        super(2);
    }

    /**
     * Test case for {@link ArpHandler#receive(PacketContext)}.
     */
    @Test
    public void testReceive() {
        VTNManagerImpl mgr = vtnMgr;
        TestStub stub = stubObj;
        ISwitchManager swMgr = mgr.getSwitchManager();
        ITopologyManager topoMgr = mgr.getTopologyManager();
        byte[] src = {(byte)0x00, (byte)0x00, (byte)0x00,
                      (byte)0x00, (byte)0x00, (byte)0x11};
        byte[] srcmc = {(byte)0x01, (byte)0x00, (byte)0x00,
                        (byte)0x00, (byte)0x00, (byte)0x11};
        byte[] dst = {(byte)0x00, (byte)0x11, (byte)0x22,
                      (byte)0x33, (byte)0x44, (byte)0x55};
        byte[] sender = {(byte)192, (byte)168, (byte)0, (byte)1};
        byte[] target = {(byte)192, (byte)168, (byte)0, (byte)250};

        Set<Node> existNodes = swMgr.getNodes();
        Set<NodeConnector> existConnectors = new HashSet<NodeConnector>();
        for (Node node : existNodes) {
            existConnectors.addAll(swMgr.getNodeConnectors(node));
        }

        Set<Node> testNodes = new HashSet<Node>(existNodes);
        testNodes.add(null);

        Set<NodeConnector> edgePorts = new HashSet<NodeConnector>();

        for (Node node : testNodes) {
            Set<NodeConnector> mappedThis = new HashSet<NodeConnector>();
            Set<NodeConnector> noMappedThis
                    = new HashSet<NodeConnector>(existConnectors);

            List<String> ncstr = new ArrayList<String>();

            for (NodeConnector nc : existConnectors) {
                if (node == null || node.equals(nc.getNode())) {
                    if (!topoMgr.isInternal(nc)) {
                        edgePorts.add(nc);
                        if (node != null) {
                            ncstr.add(nc.toString());
                        }
                        mappedThis.add(nc);
                    }
                }
            }

            swMgr.removeSubnet("test");
            SubnetConfig sconf = new SubnetConfig("test", "192.168.0.254/24", ncstr);
            swMgr.addSubnet(sconf);

            noMappedThis.removeAll(mappedThis);

            // null
            PacketResult result = mgr.receiveDataPacket(null);
            assertEquals(PacketResult.IGNORED, result);
            List<RawPacket> dataList = stub.getTransmittedDataPacket();
            assertEquals(0, dataList.size());

            // receive ARP request and ARP reply.
            // But because wait 3 seconds before receive ARP reply,
            // ARP requestor is expired.
            testReceiveDataPacketBCLoop(mgr, null, (short)0,
                    mappedThis, noMappedThis, stub);
            sleep(3000);
            testReceiveDataPacketARPReplyReceive(mgr, null, (short) 0,
                    mappedThis, noMappedThis, stub, true);

            // receive ARP request and ARP reply
            testReceiveDataPacketBCLoop(mgr, null, (short) 0,
                    mappedThis, noMappedThis, stub);

            testReceiveDataPacketARPReplyReceive(mgr, null, (short) 0,
                    mappedThis, noMappedThis, stub, false);

            // receive a packet send to already learned.
            testReceiveDataPacketBCLoopMatch(mgr, null, (short) 0,
                    mappedThis, noMappedThis, stub);

            // receive a packet send to controller.
            testReceiveDataPacketBCLoopToCont(mgr, null, (short) 0,
                    mappedThis, noMappedThis, stub);

            // receive a packet have unicast dst.
            testReceiveDataPacketUCLoop(mgr, null, (short) 0,
                    mappedThis, noMappedThis, stub);

            // receive packet form no mapped nodeconector.
            if (node != null && noMappedThis.size() != 0) {
                NodeConnector nc = noMappedThis.iterator().next();
                RawPacket inPkt = createARPRawPacket(src, dst, sender, target,
                                                     (short) -1, nc, ARP.REPLY);
                result = mgr.receiveDataPacket(inPkt);
                dataList = stub.getTransmittedDataPacket();
                String emsg = "(Node)" + node.toString()
                        + ",(NodeConnector)" + nc.toString();
                assertEquals(emsg, PacketResult.IGNORED, result);
                assertEquals(emsg, 0, dataList.size());
            }

            swMgr.removeSubnet(sconf);
        }

        // invalid data received case.
        Set<NodeConnector> mappedThis = new HashSet<NodeConnector>();
        for (NodeConnector nc : existConnectors) {
            if (!topoMgr.isInternal(nc)) {
                    mappedThis.add(nc);
            }
        }

        // subnet == null
        NodeConnector nc = existConnectors.iterator().next();
        RawPacket inPkt = createARPRawPacket(src, dst, sender, target, (short) -1,
                                             nc, ARP.REPLY);
        PacketResult result = mgr.receiveDataPacket(inPkt);
        List<RawPacket> dataList = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.IGNORED, result);
        assertEquals(0, dataList.size());

        inPkt = createIPv4RawPacket(src, dst, sender, target, (short) -1, nc);
        result = mgr.receiveDataPacket(inPkt);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.IGNORED, result);
        assertEquals(0, dataList.size());

        // set subnet configuration.
        SubnetConfig sconf = new SubnetConfig("test", "192.168.0.254/24", null);
        swMgr.addSubnet(sconf);

        // in case VLAN ID doesn't match.
        InetAddress ia
            = getInetAddressFromAddress(new byte[] { (byte) 192, (byte) 168,
                                                     (byte) 0, (byte) 254 });

        Subnet sub = swMgr.getSubnetByNetworkAddress(ia);
        sub.setVlan((short) 2);

        inPkt = createARPRawPacket(src, dst, sender, target, (short) -1,
                                   nc, ARP.REPLY);
        result = mgr.receiveDataPacket(inPkt);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.IGNORED, result);
        assertEquals(0, dataList.size());

        ia = getInetAddressFromAddress(hostIp);
        sub = swMgr.getSubnetByNetworkAddress(ia);
        sub.setVlan((short) 0);
        for (Node node : existNodes) {
            for (int i = 10; i < 11; i++) {
                // Receive an IPv4 packet to known host.
                // In this case no packet should be sent.
                nc = NodeConnectorCreator
                        .createOFNodeConnector(Short.valueOf((short) i), node);

                inPkt = createIPv4RawPacket(src, dst, sender, hostIp,
                                            (short) -1, nc);

                result = mgr.receiveDataPacket(inPkt);
                assertEquals(PacketResult.KEEP_PROCESSING, result);
                dataList = stub.getTransmittedDataPacket();
                assertEquals(0, dataList.size());
            }
        }

        // Send an IPv4 packet to unknown host.
        // In this case an ARP request must be broadcasted to edge ports.
        byte[] ipaddr = {(byte)10, (byte)0, (byte)0, (byte)9};
        Node node99 = NodeCreator.createOFNode(Long.valueOf(99L));
        nc = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"),
                                                        node99);

        inPkt = createIPv4RawPacket(src, dst, sender, ipaddr, (short)-1, nc);
        result = mgr.receiveDataPacket(inPkt);
        assertEquals(PacketResult.KEEP_PROCESSING, result);

        dataList = stub.getTransmittedDataPacket();
        assertEquals(edgePorts.size(), dataList.size());

        byte[] broadcast = {
            (byte)0xff, (byte)0xff, (byte)0xff,
            (byte)0xff, (byte)0xff, (byte)0xff
        };
        byte[] zero = {0, 0, 0, 0, 0, 0};
        byte[] controller = swMgr.getControllerMAC();

        for (RawPacket raw: dataList) {
            NodeConnector out = raw.getOutgoingNodeConnector();
            assertTrue(edgePorts.contains(out));

            Ethernet ether = (Ethernet)stubObj.decodeDataPacket(raw);
            checkOutEthernetPacket("", ether, EtherTypes.ARP,
                                   controller, broadcast, (short)0,
                                   EtherTypes.IPv4, ARP.REQUEST,
                                   controller, zero, gwIp, ipaddr);
        }

        // src is not a unicast packet.
        inPkt = createARPRawPacket(srcmc, dst, sender, target, (short) -1,
                                   nc, ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        assertEquals(mappedThis.size(), dataList.size());

        // target IP == controller && dst == unicast
        inPkt = createARPRawPacket(src, src, sender, gwIp, (short) -1,
                                   nc, ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        assertEquals(mappedThis.size(), dataList.size());

        // !isBroadCast(dst) && host.dladdr != dst
        inPkt = createARPRawPacket(src, controller, sender, gwIp, (short)-1,
                                   nc, ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        assertEquals(1, dataList.size());

        // host found and dst is not BC,
        // but dst is not match
        inPkt = createARPRawPacket(src, hostMac, sender, hostIp, (short) -1,
                                   nc, ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        assertEquals(1, dataList.size());
    }

    /**
     * Test case for {@link ArpHandler#receive(PacketContext)}.
     * Tests invalid case.
     */
    @Test
    public void testReceiveInvalidCase() {
        VTNManagerImpl mgr = vtnMgr;
        TestStub stub = stubObj;
        ISwitchManager swMgr = mgr.getSwitchManager();
        byte [] src = new byte[] { (byte) 0x00, (byte) 0x00, (byte) 0x00,
                                   (byte) 0x00, (byte) 0x00, (byte) 0x11 };
        byte [] srcmc = new byte[] { (byte) 0x01, (byte) 0x00, (byte) 0x00,
                                     (byte) 0x00, (byte) 0x00, (byte) 0x11 };
        byte [] dst = new byte[] { (byte) 0xff, (byte) 0xff, (byte) 0xff,
                                   (byte) 0xff, (byte) 0xff, (byte) 0xff };
        byte [] sender = new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) 1 };
        byte [] target = new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) 250 };

        Set<Node> existNodes = swMgr.getNodes();
        Set<NodeConnector> existConnectors = new HashSet<NodeConnector>();
        for (Node node: existNodes) {
            existConnectors.addAll(swMgr.getNodeConnectors(node));
        }

        // payload is !ARP && !IPv4
        NodeConnector nc = existConnectors.iterator().next();
        RawPacket inPkt = createARPRawPacket(src, dst, sender, target, (short) -1,
                          nc, ARP.REQUEST);
        Ethernet pkt= (Ethernet)stub.decodeDataPacket(inPkt);
        LLDP lldp = new LLDP();
        pkt.setPayload(lldp);
        pkt.setEtherType(EtherTypes.LLDP.shortValue());
        inPkt = stub.encodeDataPacket(pkt);
        inPkt.setIncomingNodeConnector(nc);
        PacketResult result = mgr.receiveDataPacket(inPkt);
        List<RawPacket> dataList = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.IGNORED, result);
        assertEquals(0, dataList.size());

        // !ARP.REQUEST && !ARP.REPLY
        inPkt = createARPRawPacket(src, dst, sender, target, (short) -1,
                                            nc, ARP.PROTO_TYPE_IP);
        result = mgr.receiveDataPacket(inPkt);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.IGNORED, result);
        assertEquals(0, dataList.size());

        // set subnet configuration.
        SubnetConfig sconf = new SubnetConfig("test", "192.168.0.254/24", null);
        swMgr.addSubnet(sconf);

        // sender == target
        inPkt = createARPRawPacket(srcmc, dst, sender, sender, (short) -1,
                                   nc, ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.IGNORED, result);
        assertEquals(0, dataList.size());

        // host found but dst is not match
        inPkt = createARPRawPacket(src, src, sender, hostIp, (short) -1, nc,
                                   ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.IGNORED, result);
        assertEquals(0, dataList.size());
    }

    /**
     * Test case for {@link ArpHandler#receive(PacketContext)}.
     * Tests invalid case.
     */
    @Test
    public void testReceiveWithoutNodes() {

        VTNManagerImpl mgr = vtnMgr;
        TestStub stub = new TestStub(0);
        mgr.setSwitchManager(stub);
        mgr.setTopologyManager(stub);
        ISwitchManager swMgr = mgr.getSwitchManager();
        ITopologyManager topoMgr = mgr.getTopologyManager();

        Set<Node> existNodes = swMgr.getNodes();
        Set<NodeConnector> existConnectors = new HashSet<NodeConnector>();
        for (Node node : existNodes) {
            existConnectors.addAll(swMgr.getNodeConnectors(node));
        }

        Set<NodeConnector> mappedThis = new HashSet<NodeConnector>();
        for (NodeConnector nc : existConnectors) {
            if (!topoMgr.isInternal(nc)) {
                    mappedThis.add(nc);
            }
        }

        byte [] src = new byte[] { (byte) 0x00, (byte) 0x00, (byte) 0x00,
                                   (byte) 0x00, (byte) 0x00, (byte) 0x11 };
        byte [] dst = new byte[] { (byte) 0xff, (byte) 0xff, (byte) 0xff,
                                   (byte) 0xff, (byte) 0xff, (byte) 0xff };
        byte [] sender = new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) 1};
        byte [] target = new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) 250};

        SubnetConfig sconf = new SubnetConfig("test", "192.168.0.254/24", null);
        swMgr.addSubnet(sconf);

        Node node = NodeCreator.createOFNode(Long.valueOf("0"));
        NodeConnector nc = NodeConnectorCreator
                .createOFNodeConnector(Short.valueOf("10"), node);
        RawPacket inPkt = createARPRawPacket(src, dst, sender, target, (short) -1,
                                             nc, ARP.REQUEST);
        PacketResult result = mgr.receiveDataPacket(inPkt);
        List<RawPacket> dataList = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        assertEquals(0, dataList.size());

        sconf = new SubnetConfig("test", "192.168.0.254/24",
                                 new ArrayList<String>());
        swMgr.addSubnet(sconf);

        inPkt = createARPRawPacket(src, dst, sender, target, (short) -1,
                                   nc, ARP.REQUEST);
        result = mgr.receiveDataPacket(inPkt);
        dataList = stub.getTransmittedDataPacket();
        assertEquals(PacketResult.KEEP_PROCESSING, result);
        assertEquals(0, dataList.size());
    }

    /**
     * test case for {@link ArpHandler#probe(HostNodeConnector)} and
     * {@link ArpHandler#find(InetAddress)}.
     */
    @Test
    public void testFindProbe() {
        VTNManagerImpl mgr = vtnMgr;
        TestStub stub = stubObj;
        ISwitchManager swMgr = mgr.getSwitchManager();
        ITopologyManager topoMgr = mgr.getTopologyManager();

        Set<Node> existNodes = swMgr.getNodes();
        Set<NodeConnector> existConnectors = new HashSet<NodeConnector>();
        for (Node node: existNodes) {
            existConnectors.addAll(swMgr.getNodeConnectors(node));
        }

        Set<Node> testNodes = new HashSet<Node>(existNodes);
        testNodes.add(null);

        InetAddress ia6 = null;
        try {
            ia6 = InetAddress.getByAddress(new byte[]{
                    (byte) 0x20, (byte) 0x01, (byte) 0x04, (byte) 0x20,
                    (byte) 0x02, (byte) 0x81, (byte) 0x10, (byte) 0x04,
                    (byte) 0xe1, (byte) 0x23, (byte) 0xe6, (byte) 0x88,
                    (byte) 0xd6, (byte) 0x55, (byte) 0xa1, (byte) 0xb0 });
        } catch (UnknownHostException e) {
            unexpected(e);
        }

        InetAddress ia
            = getInetAddressFromAddress(new byte[] { (byte) 10, (byte) 0,
                                                     (byte) 0, (byte) 1 });

        byte [] mac = new byte [] { 0x00, 0x00, 0x00, 0x11, 0x22, 0x33};
        for (Node node: testNodes) {
            String emsg = "(Node)" + ((node == null) ? "null" : node.toString());
            int numMapped = 0;
            Set<NodeConnector> mappedThis = new HashSet<NodeConnector>();
            Set<NodeConnector> noMappedThis
                    = new HashSet<NodeConnector>(existConnectors);

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
            SubnetConfig sconf = new SubnetConfig("test", "192.168.0.254/24",
                                                  ncstr);
            swMgr.addSubnet(sconf);

            noMappedThis.removeAll(mappedThis);
            numMapped = mappedThis.size();

            mgr.find(ia);

            List<RawPacket> dataList = stub.getTransmittedDataPacket();
            assertEquals(numMapped, dataList.size());

            for (RawPacket raw : dataList) {
                Packet pkt = stub.decodeDataPacket(raw);
                assertTrue(raw.getOutgoingNodeConnector().toString(),
                        mappedThis.contains(raw.getOutgoingNodeConnector()));
                assertFalse(raw.getOutgoingNodeConnector().toString(),
                        noMappedThis.contains(raw.getOutgoingNodeConnector()));

                Ethernet eth = (Ethernet)pkt;

                checkOutEthernetPacket(
                        emsg + "(Out nc)" + raw.getOutgoingNodeConnector().toString(),
                        eth, EtherTypes.ARP,
                        swMgr.getControllerMAC(),
                        new byte[] {-1, -1, -1, -1, -1, -1},
                        (short)0, EtherTypes.IPv4, ARP.REQUEST,
                        swMgr.getControllerMAC(),
                        new byte[] {0, 0, 0, 0, 0, 0},
                        gwIp, ia.getAddress());
            }

            mgr.find(ia6);
            dataList = stub.getTransmittedDataPacket();
            assertEquals(0, dataList.size());

            // test probe
            Node hnode = NodeCreator.createOFNode(0L);
            NodeConnector nc
                = NodeConnectorCreator.createOFNodeConnector(Short.valueOf("10"), hnode);
            HostNodeConnector host = null;
            HostNodeConnector hostnull = null;
            try {
                host = new HostNodeConnector(mac, ia, nc, (short) 0);
                hostnull = new HostNodeConnector(mac, ia, null, (short) 0);
            } catch (ConstructionException e) {
                unexpected(e);
            }

            mgr.probe(host);

            dataList = stub.getTransmittedDataPacket();
            assertEquals(1, dataList.size());

            RawPacket raw = dataList.get(0);
            assertEquals(nc, raw.getOutgoingNodeConnector());

            Ethernet eth = (Ethernet)stub.decodeDataPacket(raw);
            checkOutEthernetPacket("", eth, EtherTypes.ARP,
                    swMgr.getControllerMAC(), mac,
                    (short)0, EtherTypes.IPv4, ARP.REQUEST,
                    swMgr.getControllerMAC(), mac,
                    gwIp, ia.getAddress());

            mgr.probe(hostnull);
            dataList = stub.getTransmittedDataPacket();
            assertEquals(0, dataList.size());

            swMgr.removeSubnet(sconf);

            // subnet == null
            mgr.find(ia);
            dataList = stub.getTransmittedDataPacket();
            assertEquals(0, dataList.size());

            mgr.probe(host);
            dataList = stub.getTransmittedDataPacket();
            assertEquals(0, dataList.size());
        }

        // if null
        mgr.probe(null);
        List<RawPacket> dataList = stub.getTransmittedDataPacket();
        assertEquals(0, dataList.size());
    }


    // private methods

    private void testReceiveDataPacketBCLoop(VTNManagerImpl mgr, VBridgePath bpath,
            short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub) {
         testReceiveDataPacketAHCommonLoop(mgr, bpath, vlan,
                 mappedThis, noMappedThis, stub, (short)1, EtherTypes.ARP,
                 ARP.REQUEST, null, testTgtIp, false);
    }

    private void testReceiveDataPacketUCLoop(VTNManagerImpl mgr, VBridgePath bpath,
            short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub) {
        testReceiveDataPacketAHCommonLoop(mgr, bpath, vlan,
                mappedThis, noMappedThis, stub, (short)0, EtherTypes.IPv4,
                (short)-1, null, testTgtIp, false);
    }

    private void testReceiveDataPacketBCLoopMatch(VTNManagerImpl mgr, VBridgePath bpath,
            short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub) {
         testReceiveDataPacketAHCommonLoop(mgr, bpath, vlan,
                 mappedThis, noMappedThis, stub, (short)1, EtherTypes.ARP,
                 ARP.REQUEST, null, hostIp, false);
     }

    private void testReceiveDataPacketBCLoopToCont(VTNManagerImpl mgr, VBridgePath bpath,
            short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub) {
         testReceiveDataPacketAHCommonLoop(mgr, bpath, vlan,
                 mappedThis, noMappedThis, stub, (short)1, EtherTypes.ARP,
                 ARP.REQUEST, null, gwIp, false);
     }

    private void testReceiveDataPacketARPReplyReceive(VTNManagerImpl mgr, VBridgePath bpath,
            short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub, boolean expired) {
         testReceiveDataPacketAHCommonLoop(mgr, bpath, vlan,
                 mappedThis, noMappedThis, stub, (short)1, EtherTypes.ARP,
                 ARP.REPLY, null, testTgtIp, expired);
     }

    /**
     * common method for {@link VTNManagerImpl#receiveDataPacket(RawPacket)}.
     * This tests with using {@link ArpHandler}.
     */
    private void testReceiveDataPacketAHCommonLoop(VTNManagerImpl mgr, VBridgePath bpath,
            short vlan, Set<NodeConnector> mappedThis, Set<NodeConnector> noMappedThis,
            TestStub stub, short isBc, EtherTypes ethType, short arpType,
            NodeConnector targetnc, byte[] targetIp, boolean expired) {

        ISwitchManager swMgr = mgr.getSwitchManager();
        List<EthernetAddress> ethers = createEthernetAddresses(false);
        byte[] src = null;
        byte[] dst = null;
        byte[] sender = targetIp;
        byte[] target = targetIp;

        if (arpType == ARP.REPLY) {
            src = new byte[] { (byte) 0x00, (byte) 0x00, (byte) 0x00,
                               (byte) 0xff, (byte) 0xff, (byte) 0x11 };
            sender = targetIp;
        } else {
            if (isBc > 0) {
                dst = new byte[] { (byte) 0xff, (byte) 0xff, (byte) 0xff,
                                   (byte) 0xff, (byte) 0xff, (byte) 0xff };
            } else {
                dst = new byte[] { (byte) 0x00, (byte) 0x00, (byte) 0x00,
                                   (byte) 0xff, (byte) 0xff, (byte) 0x11 };
            }
        }

        boolean isFirst = true;
        for (NodeConnector nc: mappedThis) {
            if (targetnc != null && !targetnc.equals(nc)) {
                continue;
            }
            byte iphost = 1;
            for (EthernetAddress ea: ethers) {
                byte[] bytes = ea.getValue();

                if (arpType == ARP.REPLY) {
                    // in case test with ARP.REPLY packet,
                    // revese MAC address and IP address.
                    dst = new byte[] { bytes[0], bytes[1], bytes[2],
                                       bytes[3], bytes[4], bytes[5] };
                    target = new byte[] { (byte) 192, (byte) 168,
                                          (byte) 0, (byte) iphost };
                } else {
                    src = new byte[] { bytes[0], bytes[1], bytes[2],
                                       bytes[3], bytes[4], bytes[5] };
                    sender = new byte[] { (byte) 192, (byte) 168,
                                          (byte) 0, (byte) iphost };
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

                // check result
                if (nc != null &&
                        nc.getType() == NodeConnector.NodeConnectorIDType.OPENFLOW) {
                    String emsg = "(In nc)" + nc.toString()
                            + ",(EthernetAddress)" + ea.toString();

                    if (isFirst && !expired) {
                        assertEquals(emsg, PacketResult.KEEP_PROCESSING, result);
                    } else {
                        // in case packet is ARP.Reply and there aren't ARP requester,
                        // received packet is IGNORED.
                        assertEquals(emsg, PacketResult.IGNORED, result);
                    }

                    Packet payload = inPktDecoded.getPayload();
                    if (inPktDecoded.getEtherType() == EtherTypes.VLANTAGGED.shortValue()) {
                        payload = payload.getPayload();
                    }

                    List<RawPacket> dataList = stub.getTransmittedDataPacket();
                    if ((Arrays.equals(target, hostIp) || Arrays.equals(target, gwIp)) &&
                            arpType != ARP.REPLY) {
                        assertEquals(emsg, 1, dataList.size());
                    } else if (ethType.shortValue() == EtherTypes.ARP.shortValue() &&
                            arpType == ARP.REPLY) {
                        if (isFirst && !expired) {
                            assertEquals(emsg,
                                mappedThis.size() * ethers.size(), dataList.size());
                            isFirst = false;
                        } else {
                            // in case arpType == ARP.Reply
                            // send ARP.Reply packet at first time only.
                            assertEquals(emsg, 0, dataList.size());
                        }
                    } else {
                        assertEquals(emsg, mappedThis.size(), dataList.size());
                    }

                    for (RawPacket raw: dataList) {
                        String emsgRaw = emsg
                                + ",(Out nc)" + raw.getOutgoingNodeConnector();
                        Packet pkt = stub.decodeDataPacket(raw);
                        assertTrue(emsgRaw,
                                mappedThis.contains(raw.getOutgoingNodeConnector()));
                        assertFalse(emsgRaw,
                                noMappedThis.contains(raw.getOutgoingNodeConnector()));

                        if (Arrays.equals(target, hostIp) || Arrays.equals(target, gwIp)) {
                            assertEquals(emsgRaw, nc, raw.getOutgoingNodeConnector());
                        }

                        if (ethType.shortValue() == EtherTypes.IPv4.shortValue()) {
                            // send ARP Request
                            checkOutEthernetPacket(emsgRaw,
                                    (Ethernet)pkt, EtherTypes.ARP,
                                    swMgr.getControllerMAC(),
                                    new byte[] {-1, -1, -1, -1, -1, -1},
                                    vlan, EtherTypes.IPv4, ARP.REQUEST,
                                    swMgr.getControllerMAC(),
                                    new byte[] {0, 0, 0, 0, 0, 0},
                                    gwIp, target);
                        } else if (ethType.shortValue() == EtherTypes.ARP.shortValue() &&
                                arpType == ARP.REQUEST && Arrays.equals(target, hostIp) ){
                            // in this case receive ARP Reply
                            checkOutEthernetPacket(emsgRaw,
                                    (Ethernet)pkt, EtherTypes.ARP,
                                    hostMac, src, vlan,
                                    EtherTypes.IPv4, ARP.REPLY,
                                    hostMac, src, target, sender);
                        } else if (ethType.shortValue() == EtherTypes.ARP.shortValue() &&
                                arpType == ARP.REQUEST && Arrays.equals(target, gwIp)){
                            // in this case receive ARP Reply
                            checkOutEthernetPacket(emsgRaw,
                                    (Ethernet)pkt, EtherTypes.ARP,
                                    swMgr.getControllerMAC(),
                                    src, vlan,
                                    EtherTypes.IPv4, ARP.REPLY,
                                    swMgr.getControllerMAC(),
                                    src,
                                    target, sender);
                        } else if (ethType.shortValue() == EtherTypes.ARP.shortValue() &&
                                    arpType == ARP.REQUEST){
                            // in this case flooded
                            checkOutEthernetPacket(emsgRaw,
                                    (Ethernet)pkt, EtherTypes.ARP,
                                    swMgr.getControllerMAC(), dst, vlan,
                                    EtherTypes.IPv4, ARP.REQUEST,
                                    swMgr.getControllerMAC(),
                                    new byte[] {0, 0, 0, 0, 0, 0},
                                    gwIp, target);
                        } else if (ethType.shortValue() == EtherTypes.ARP.shortValue() &&
                                    arpType == ARP.REPLY){
                            // in this case flooded
                            checkOutEthernetPacket(emsgRaw,
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
                        assertEquals(nc.toString() + "," + ea.toString(),
                                     PacketResult.IGNORED, result);
                    } else {
                        assertEquals(ea.toString(), PacketResult.IGNORED, result);
                    }
                }
                iphost++;
            }
        }
    }
}
