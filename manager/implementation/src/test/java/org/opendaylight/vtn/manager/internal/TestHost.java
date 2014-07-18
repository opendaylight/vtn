/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.util.HashSet;
import java.util.Set;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacTableEntry;
import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.cluster.MapReference;
import org.opendaylight.vtn.manager.internal.cluster.PortVlan;

import org.opendaylight.controller.hosttracker.HostIdFactory;
import org.opendaylight.controller.hosttracker.IHostId;
import org.opendaylight.controller.hosttracker.IfIptoHost;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.RawPacket;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * An instance of this class represents a host attached to the IP network.
 */
public class TestHost extends TestBase {
    /**
     * A long integer value which represents the MAC address of this host.
     */
    private final long  macAddress;

    /**
     * An IP address assigned to this host.
     */
    private final InetAddress  inetAddress;

    /**
     * A {@link PortVlan} instance which represents the VLAN network to which
     * this host belongs.
     */
    private final PortVlan  portVlan;

    /**
     * A reference to the virtual mapping which maps this host.
     *
     * <p>
     *   Note that this field does not affect object identity.
     * </p>
     */
    private MapReference  mapping;

    /**
     * Construct a new instance.
     *
     * @param mac   A long value which indicates the MAC address.
     * @param vlan  A VLAN ID.
     * @param addr  An array of bytes which represents an IP address.
     * @param nc    A {@link NodeConnector} instance corresponding to the
     *              switch port to which this host is connected.
     */
    public TestHost(long mac, short vlan, byte[] addr, NodeConnector nc) {
        try {
            macAddress = mac;
            portVlan = new PortVlan(nc, vlan);
            inetAddress = InetAddress.getByAddress(addr);
        } catch (Exception e) {
            throw new AssertionError("Unexpeced exception: " + e, e);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param mac     A long value which indicates the MAC address.
     * @param pvlan   A {@link PortVlan} instance which represents the VLAN
     *                network to which the host is connected.
     * @param ipaddr   A {@link InetAddress} instance which represents an
     *                IP address assigned to this host.
     */
    public TestHost(long mac, PortVlan pvlan, InetAddress ipaddr) {
        macAddress = mac;
        portVlan = pvlan;
        inetAddress = ipaddr;
    }

    /**
     * Return a long value which represents the MAC address of this host.
     * this host.
     *
     * @return  A long vlaue which represents the MAC address.
     */
    public long getMacAddress() {
        return macAddress;
    }

    /**
     * Return a VLAN ID of this host.
     *
     * @return  A VLAN ID.
     */
    public short getVlan() {
        return portVlan.getVlan();
    }

    /**
     * Return a {@link L2Host} instance which represents the location of
     * this host.
     *
     * @return  A {@link L2Host} instance.
     */
    public L2Host getL2Host() {
        return new L2Host(macAddress, portVlan.getVlan(),
                          portVlan.getNodeConnector());
    }

    /**
     * Return a {@link DataLinkHost} instance which represents the location of
     * this host.
     *
     * @return  A {@link DataLinkHost} instance.
     */
    public DataLinkHost getDataLinkHost() {
        EthernetAddress eaddr = getEthernetAddress();
        return new EthernetHost(eaddr, portVlan.getVlan());
    }

    /**
     * Return a {@link NodeConnector} instance corresponding to a swtich port
     * to which this host is connected.
     *
     * @return  A {@link NodeConnector} instance.
     */
    public NodeConnector getPort() {
        return portVlan.getNodeConnector();
    }

    /**
     * Return a {@link Node} instance corresponding to a swtich to which
     * this host is connected.
     *
     * @return  A {@link Node} instance.
     */
    public Node getNode() {
        return portVlan.getNodeConnector().getNode();
    }

    /**
     * Return a {@link PortVlan} instance to which this host belongs.
     *
     * @return  A {@link PortVlan} instance.
     */
    public PortVlan getPortVlan() {
        return portVlan;
    }

    /**
     * Return a {@link MacVlan} instance which represents this host.
     *
     * @return  A {@link MacVlan} instance.
     */
    public MacVlan getMacVlan() {
        return new MacVlan(macAddress, portVlan.getVlan());
    }

    /**
     * Return a reference to the virtual mappin which maps this host.
     *
     * @return  A {@link MapReference} instance if this host is mapped by a
     *          virtual mapping. Otherwise {@code null}.
     */
    public MapReference getMapping() {
        return mapping;
    }

    /**
     * Set a reference to the virtual mapping which maps this host.
     *
     * @param ref  A reference to the virtual mapping which maps this host.
     */
    public void setMapping(MapReference ref) {
        mapping = ref;
    }

    /**
     * Determine whether both the specified host and this host are connected
     * to the same VLAN network on a switch port or not.
     *
     * @param host  A host to be tested.
     * @return  {@code true} only if the specified host is connected to
     *          the same VLAN network on a switch port.
     */
    public boolean isSameVlan(TestHost host) {
        return portVlan.equals(host.portVlan);
    }

    /**
     * Create an unicast packet which contains an ARP request which probes
     * the specified host.
     *
     * @param host  A {@code TestHost} instance.
     * @return  A {@link RawPacket} instance which contains an ARP request.
     */
    public RawPacket createArp(TestHost host) {
        byte[] dst = NetUtils.longToByteArray6(host.macAddress);
        byte[] dstIp = host.inetAddress.getAddress();
        return createArp(dst, dstIp);
    }

    /**
     * Create a broadcast packet which contains an ARP request which probes
     * the specified IP address.
     *
     * @param addr  An array of bytes which represents an IP address.
     * @return  A {@link RawPacket} instance which contains an ARP request.
     */
    public RawPacket createArp(byte[] addr) {
        byte[] dst = NetUtils.getBroadcastMACAddr();
        return createArp(dst, addr);
    }

    /**
     * Return a {@link EthernetAddress} instance which represents the
     * MAC address of this host.
     *
     * @return  A {@link EthernetAddress} instance.
     */
    public EthernetAddress getEthernetAddress() {
        byte[] addr = NetUtils.longToByteArray6(macAddress);
        EthernetAddress eaddr = null;
        try {
            eaddr = new EthernetAddress(addr);
        } catch (Exception e) {
            unexpected(e);
        }

        return eaddr;
    }

    /**
     * Return a {@link MacAddressEntry} instance which represents this host.
     *
     * @return  A {@link MacAddressEntry} instance.
     */
    public MacAddressEntry getMacAddressEntry() {
        EthernetAddress eaddr = getEthernetAddress();
        NodeConnector port = portVlan.getNodeConnector();
        short vlan = portVlan.getVlan();
        Set<InetAddress> ipSet = new HashSet<InetAddress>();
        ipSet.add(inetAddress);

        return new MacAddressEntry(eaddr, vlan, port, ipSet);
    }

    /**
     * Determine whether the MAC address of this host is learned correctly
     * or not.
     *
     * @param mgr   VTN Manager service.
     * @param path  A path to the vBridge.
     */
    public void checkLearned(VTNManagerImpl mgr, VBridgePath path) {
        MacTableEntry tent = getMacTableEntry(mgr, path, macAddress);
        if (mapping == null) {
            assertEquals(null, tent);
            return;
        }

        VBridgePath mapPath = mapping.getPath();
        if (!path.contains(mapPath)) {
            assertEquals(null, tent);
            return;
        }

        NodeConnector port = portVlan.getNodeConnector();
        short vlan = portVlan.getVlan();
        assertEquals(macAddress, tent.getMacAddress());
        assertEquals(vlan, tent.getVlan());
        assertEquals(port, tent.getPort());
        assertEquals(mapPath, tent.getEntryId().getMapPath());

        Set<InetAddress> ipSet = tent.getInetAddresses();
        assertEquals(1, ipSet.size());
        InetAddress ipaddr = ipSet.iterator().next();
        assertEquals(inetAddress, ipaddr);

        // Ensure that this host is learned by the host tracker.
        IfIptoHost ht = mgr.getHostTracker();
        EthernetAddress eaddr = getEthernetAddress();
        IHostId id = HostIdFactory.create(ipaddr, eaddr);
        HostNodeConnector hnc = ht.hostQuery(id);
        assertNotNull(hnc);
        assertEquals(port, hnc.getnodeConnector());
        assertEquals(vlan, hnc.getVlan());
        assertEquals(eaddr, hnc.getDataLayerAddress());
        assertEquals(ipaddr, hnc.getNetworkAddress());
        assertFalse(hnc.isStaticHost());
    }

    /**
     * Create a new instance which represents this host connected to the
     * specified port.
     *
     * @param port  A {@link NodeConnector} instance corresponding to a
     *              switch port.
     * @return  A new {@link TestHost} instance.
     */
    public TestHost moveTo(NodeConnector port) {
        PortVlan pvlan = new PortVlan(port, portVlan.getVlan());
        TestHost host = new TestHost(macAddress, pvlan, inetAddress);
        host.mapping = this.mapping;
        return host;
    }

    /**
     * Create a packet which contains an ARP request.
     *
     * @param dst   An array of bytes which represents the target MAC address.
     * @param addr  An arrya of bytes which represents the target IP address.
     * @return  A {@link RawPacket} instance which contains an ARP request.
     */
    private RawPacket createArp(byte[] dst, byte[] addr) {
        byte[] src = NetUtils.longToByteArray6(macAddress);
        byte[] srcIp = inetAddress.getAddress();
        short vlan  = portVlan.getVlan();
        if (vlan == 0) {
            vlan = -1;
        }

        NodeConnector port = portVlan.getNodeConnector();
        return createARPRawPacket(src, dst, srcIp, addr, vlan, port,
                                  ARP.REQUEST);
    }

    /**
     * Return a MAC address table entry which represents this host.
     *
     * @param mgr   VTN Manager service.
     * @param path  A path to the vBridge.
     * @param mac   A long value which represents the MAC address of this host.
     */
    private MacTableEntry getMacTableEntry(VTNManagerImpl mgr,
                                           VBridgePath path, long mac) {
        MacAddressTable table = mgr.getMacAddressTable(path);
        if (table == null) {
            return null;
        }

        Long key = Long.valueOf(mac);
        return table.getEntry(key);
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!(o instanceof TestHost)) {
            return false;
        }

        TestHost host = (TestHost)o;
        return (macAddress == host.macAddress &&
                inetAddress.equals(host.inetAddress) &&
                portVlan.equals(host.portVlan));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = (int)(macAddress ^ (macAddress >>> 32));
        return h ^ inetAddress.hashCode() ^ portVlan.hashCode();
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("TestHost[");
        builder.append("mvlan=").
            append(MiscUtils.formatMacAddress(macAddress)).
            append(", ipaddr=").append(inetAddress).
            append(", pvlan=").append(portVlan).append(']');

        return builder.toString();
    }
}
