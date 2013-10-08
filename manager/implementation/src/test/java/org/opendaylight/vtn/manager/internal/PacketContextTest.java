/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal;

import java.io.File;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Hashtable;
import java.util.List;
import java.util.TreeMap;

import org.apache.felix.dm.impl.ComponentImpl;
import org.junit.Test;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IEEE8021Q;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.PacketException;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.EtherTypes;
import org.opendaylight.controller.sal.utils.GlobalConstants;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;

/**
 * JUnit test for {@link PacketContext}
 */
public class PacketContextTest extends TestBase {
    private VTNManagerImpl vtnMgr = null;
    private GlobalResourceManager resMgr = null;
    private TestStub stubObj = null;

    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        short vlans[] = { -10, 0, 1, 100, 4095 };
        Ethernet ether;
        byte[] dst = new byte[] { (byte) 0xff, (byte) 0xff, (byte) 0xff,
                                  (byte) 0xff, (byte) 0xff, (byte) 0xff };
        byte[] target = new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) 250 };

        for (NodeConnector nc : createNodeConnectors(3, false)) {
            byte iphost = 1;
            for (short vlan : vlans) {
                for (EthernetAddress ea : createEthernetAddresses(false)) {
                    byte[] bytes = ea.getValue();
                    byte[] src = new byte[] { bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5] };
                    byte[] sender = new byte[] { (byte) 192, (byte) 168, (byte) 0, iphost };
                    byte[] sender2 = new byte[] { (byte) 192, (byte) 168, (byte) 0, (byte) (iphost + 100) };

                    testPacketContext(src, dst, sender, target, vlan, nc);

                    ether = createARPPacket(src, dst, sender2, target, (vlan > 0) ? vlan : -1, ARP.REQUEST);
                    testPacketContext(ether, src, dst, sender2, target, vlan, nc);

                    iphost++;
                }
            }

        }
    }

    /**
     * Test case for {@link PacketContext(RawPacket raw, Ethernet ether)}
     *
     * @param src       src MAC address.
     * @param dst       dst MAC address.
     * @param sender    sender IP address.
     * @param target    target IP address.
     * @param vlan      VLAN ID
     * @param nc        Node Connector.
     */
    private void testPacketContext(byte[] src, byte[] dst, byte[] sender, byte[] target, short vlan, NodeConnector nc) {
        InetAddress ipaddr = null;
        MacTableEntry me;
        PortVlan pv;
        String desc;
        String msg = createErrorMessageString(src, dst, sender, target, vlan, nc);

        // createARPPacketContext create PacketContext
        // by using {@link PacketContext(RawPacket, Ethernet)}.
        PacketContext pc = createARPPacketContext(src, dst, sender, target,
                                                (vlan > 0) ? vlan : -1, nc, ARP.REQUEST);

        VBridgePath path = new VBridgePath("tenant1", "bridge1");
        Long key = NetUtils.byteArray6ToLong(src);
        // test getter methods
        if (vlan <= 0) {
            assertEquals(msg, 0, pc.getVlan());
            pv = new PortVlan(nc, (short) 0);
            me = new MacTableEntry(path, key, nc, (short) 0, ipaddr);
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

        pc.addObsoleteEntry(key, me);

        TreeMap<Long, MacTableEntry> map = new TreeMap<Long, MacTableEntry>();
        map.put(key, me);

        assertEquals(msg, map, pc.getObsoleteEntries());

        // test getFrame()
        Ethernet ether = pc.getFrame();
        checkOutEthernetPacket("", ether, EtherTypes.ARP, src, dst, vlan, EtherTypes.IPv4,
                ARP.REQUEST, src, dst, sender, target);

        // test createFrame()
        Ethernet newether;
        if (vlan <= 0) {
            desc = convertForDescription(ether, nc, (short) 0);
            newether = pc.createFrame((short) 0);
        } else {
            desc = convertForDescription(ether, nc, vlan);
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
        desc = convertForDescription(ether, nc, newvlan);
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
        desc = convertForDescription(ether, nc, newvlan);
        newether = pctxp.createFrame(newvlan);
        assertEquals(msg, desc, pctxp.getDescription(nc));
        arp = getPayload(newether, EtherTypes.ARP, (vlan > 0) ? vlan : -1, msg);
        assertNull(msg, arp);
        assertArrayEquals(msg, src, newether.getSourceMACAddress());
        assertArrayEquals(msg, dst, newether.getDestinationMACAddress());
        assertNotNull(msg, newether);
    }

    /**
     * Test case for {@link PacketContext(RawPacket raw, Ethernet ether)}
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
            me = new MacTableEntry(path, key, nc, (short) 0, ipaddr);
        } else {
            assertEquals(vlan, pctx.getVlan());
            me = new MacTableEntry(path, key, nc, vlan, ipaddr);
        }

        pctx.addObsoleteEntry(key, me);

        TreeMap<Long, MacTableEntry> map = new TreeMap<Long, MacTableEntry>();
        map.put(key, me);

        assertEquals(msg, map, pctx.getObsoleteEntries());

        // test getFrame()
        assertEquals(ether, pctx.getFrame());
        checkOutEthernetPacket(msg, ether, EtherTypes.ARP, src, dst, vlan, EtherTypes.IPv4,
                ARP.REQUEST, src, dst, sender, target);

        // test createFrame()
        Ethernet newether;
        if (vlan < 0) {
            desc = convertForDescription(ether, nc, (short) 0);
            newether = pctx.createFrame((short) 0);
        } else {
            desc = convertForDescription(ether, nc, vlan);
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
            vlantag.setCfi((byte) 0x0).setPcp((byte) 0x0).setVid((short)vlan).
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
        startVTNManagerAfterRemoveConfigfiles();

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
        NodeConnector ncOut
            = NodeConnectorCreator.createOFNodeConnector(Short.valueOf((short)11), node);

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

        stopVTNManagerAndRemoveConfigfiles();
    }


    // private methods

    /**
     * Create a string to compare description.
     *
     * @param ether a Ethernet Object
     * @param port  a NodeConnector
     * @param vlan  VLAN ID
     * @return  A brief description of the specified ethernet frame.
     */
    private String convertForDescription(Ethernet ether, NodeConnector port, short vlan) {
        String srcmac = HexEncode.bytesToHexStringFormat(ether.getSourceMACAddress());
        String dstmac = HexEncode.bytesToHexStringFormat(ether.getDestinationMACAddress());
        int type = ether.getEtherType() & 0xffff;

        StringBuilder builder = new StringBuilder("src=");
        builder.append(srcmac).append(", dst=")
               .append(dstmac).append(", port=")
               .append(port).append(", type=0x")
               .append(Integer.toHexString(type))
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
     * start VTNManager after remove configuration files.
     */
    private void startVTNManagerAfterRemoveConfigfiles() {
        setupStartupDir();

        vtnMgr = new VTNManagerImpl();
        resMgr = new GlobalResourceManager();
        ComponentImpl c = new ComponentImpl(null, null, null);
        stubObj = new TestStub(2);

        Hashtable<String, String> properties = new Hashtable<String, String>();
        properties.put("containerName", "default");
        c.setServiceProperties(properties);

        resMgr.setClusterGlobalService(stubObj);
        resMgr.init(c);
        vtnMgr.setResourceManager(resMgr);
        vtnMgr.setClusterContainerService(stubObj);
        vtnMgr.setSwitchManager(stubObj);
        vtnMgr.setTopologyManager(stubObj);
        vtnMgr.setDataPacketService(stubObj);

        vtnMgr.setConnectionManager(stubObj);
        vtnMgr.init(c);
        vtnMgr.clearDisabledNode();
    }

    /**
     * stop VTNManager and remove configuration files.
     */
    private void stopVTNManagerAndRemoveConfigfiles() {
        vtnMgr.stopping();
        vtnMgr.stop();
        vtnMgr.destroy();
        resMgr.destroy();

        cleanupStartupDir();
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
}