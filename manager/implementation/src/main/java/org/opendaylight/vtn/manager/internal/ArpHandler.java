/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Arrays;
import java.util.Iterator;
import java.util.Set;
import java.util.HashSet;
import java.util.Timer;
import java.util.TimerTask;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.TimeUnit;
import java.net.InetAddress;
import java.net.Inet4Address;
import java.net.UnknownHostException;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.cluster.ObjectPair;

import org.opendaylight.controller.hosttracker.IfIptoHost;
import org.opendaylight.controller.hosttracker.hostAware.HostNodeConnector;
import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.ARP;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.IPv4;
import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.packet.PacketResult;
import org.opendaylight.controller.sal.utils.HexEncode;
import org.opendaylight.controller.sal.utils.NetUtils;
import org.opendaylight.controller.switchmanager.ISwitchManager;
import org.opendaylight.controller.switchmanager.Subnet;

/**
 * ARP handler emulator.
 *
 * <p>
 *   If no VTN exists in the container, this class is used to emulate
 *   arphandler in the base controller.
 * </p>
 */
public class ArpHandler {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(ArpHandler.class);

    /**
     * ARP request timeout in seconds.
     */
    private static final short  ARP_REQUEST_TIMEOUT = 1;

    /**
     * The number of milliseconds in {@link #ARP_REQUEST_TIMEOUT}.
     */
    private static final long  ARP_REQUEST_TIMEOUT_MSEC =
        TimeUnit.SECONDS.toMillis((long)ARP_REQUEST_TIMEOUT);

    /**
     * VTN Manager service.
     */
    private final VTNManagerImpl  vtnManager;

    /**
     * Timer thread to detect timeout of ARP requests.
     */
    private final Timer  timerThread;

    /**
     * Ongoing ARP requests.
     */
    private final ConcurrentMap<ObjectPair<InetAddress, Short>, Requestor>
        arpRequestors =
        new ConcurrentHashMap<ObjectPair<InetAddress, Short>, Requestor>();

    /**
     * Keep hosts waiting for ARP reply.
     */
    private final class Requestor {
        /**
         * Set of hosts waiting for ARP reply.
         */
        private Set<HostNodeConnector>  hosts =
            new HashSet<HostNodeConnector>();

        /**
         * Count down timer.
         */
        private short  timer = ARP_REQUEST_TIMEOUT * 2;

        /**
         * Create a new requestor object.
         *
         * @param host  A host to be added to the requestor set.
         */
        private Requestor(HostNodeConnector host) {
            hosts.add(host);
        }

        /**
         * Add a host to the set of requestors.
         *
         * @param host  A host to be added to the requestor set.
         */
        private void add(HostNodeConnector host) {
            hosts.add(host);
        }

        /**
         * Return a set of requestors.
         *
         * @return  A set of requestors.
         */
        private Set<HostNodeConnector> getHosts() {
            return hosts;
        }

        /**
         * Decrement count down timer.
         *
         * @return  {@code true} is returned if the request has expired.
         */
        private boolean decrementTimer() {
            timer--;
            return (timer == 0);
        }
    }

    /**
     * Construct a new ARP handler emulator.
     *
     * @param mgr  VTN Manager service.
     */
    ArpHandler(VTNManagerImpl mgr) {
        String containerName = mgr.getContainerName();
        if (LOG.isTraceEnabled()) {
            LOG.trace("{}: Initializing ArpHandler", containerName);
        }
        vtnManager = mgr;

        // Initialize timeout timer task.
        timerThread = new Timer(containerName + ": VTN ArpHandler Timer");
        TimerTask task = new TimerTask() {
            /**
             * Scan ARP requests and detect expired requests.
             */
            @Override
            public void run() {
                expireRequests();
            }
        };

        timerThread.scheduleAtFixedRate(task, ARP_REQUEST_TIMEOUT_MSEC,
                                        ARP_REQUEST_TIMEOUT_MSEC);
    }

    /**
     * Destroy ARP handler emulator.
     */
    void destroy() {
        if (LOG.isTraceEnabled()) {
            LOG.trace("{}: Destroying ArpHandler",
                      vtnManager.getContainerName());
        }
        timerThread.cancel();
    }

    /**
     * Handler for receiving the packet.
     *
     * @param pctx  The context of the received packet.
     * @return  An indication if the packet should still be processed or
     *          we should stop it.
     */
    PacketResult receive(PacketContext pctx) {
        Packet payload = pctx.getPayload();
        if (payload instanceof ARP) {
            return handlePacket(pctx, (ARP)payload);
        } else if (payload instanceof IPv4) {
            return handlePacket(pctx, (IPv4)payload);
        }

        return PacketResult.IGNORED;
    }

    /**
     * This method initiates the discovery of a host based on its IP address.
     * This is triggered by query of an application to the HostTracker. The
     * requested IP address doesn't exist in the local database at this point.
     *
     * @param addr  IP Address encapsulated in InetAddress class
     */
    void find(InetAddress addr) {
        if (LOG.isDebugEnabled()) {
            LOG.debug("{}: find() called: addr={}",
                      vtnManager.getContainerName(), addr);
        }

        if (!(addr instanceof Inet4Address)) {
            LOG.debug("{}: IPv4 address must be specified: {}",
                      vtnManager.getContainerName(), addr);
            return;
        }

        ISwitchManager swMgr = vtnManager.getSwitchManager();
        Subnet subnet = swMgr.getSubnetByNetworkAddress(addr);
        if (subnet == null) {
            LOG.debug("{}: Can't find subnet matching IP {}",
                      vtnManager.getContainerName(), addr);
            return;
        }

        // Send a broadcast ARP request to this subnet.
        floodArpRequest(addr, subnet);
    }

    /**
     * This method is called by HostTracker to see if a learned Host is still
     * in the network. Used mostly for ARP Aging.
     *
     * @param host  The Host that needs to be probed
     */
    void probe(HostNodeConnector host) {
        if (LOG.isDebugEnabled()) {
            LOG.debug("{}: probe() called: host={}",
                      vtnManager.getContainerName(), host);
        }

        if (host == null) {
            LOG.debug("{}: probe(): host is null",
                      vtnManager.getContainerName());
            return;
        }

        InetAddress addr = host.getNetworkAddress();
        ISwitchManager swMgr = vtnManager.getSwitchManager();
        Subnet subnet = swMgr.getSubnetByNetworkAddress(addr);
        if (subnet == null) {
            LOG.debug("{}: Can't find subnet matching IP {}",
                      vtnManager.getContainerName(), addr);
            return;
        }

        NodeConnector port = host.getnodeConnector();
        if (port == null) {
            LOG.error("{}: Can't send ARP request: No port: {}",
                      vtnManager.getContainerName(), host);
            return;
        }

        // Send an unicast ARP request to this host.
        byte[] src = swMgr.getControllerMAC();
        byte[] dst = host.getDataLayerAddressBytes();
        byte[] spa = subnet.getNetworkAddress().getAddress();
        byte[] tpa = host.getNetworkAddress().getAddress();
        short vlan = subnet.getVlan();

        sendArp(port, ARP.REQUEST, src, dst, spa, tpa, vlan);
    }

    /**
     * Handle received ARP packet.
     *
     * @param pctx  The context of the received packet.
     * @param arp   ARP packet in the received packet.
     * @return  An indication if the packet should still be processed or
     *          we should stop it.
     */
    private PacketResult handlePacket(PacketContext pctx, ARP arp) {
        short op = arp.getOpCode();
        if (op != ARP.REQUEST && op != ARP.REPLY) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: Ignore unsupported ARP packet: {}",
                          vtnManager.getContainerName(), op);
            }
            return PacketResult.IGNORED;
        }

        InetAddress sdrIp;
        byte[] spa = arp.getSenderProtocolAddress();
        try {
            sdrIp = InetAddress.getByAddress(spa);
        } catch (UnknownHostException e) {
            LOG.error("{}: Ignore broken ARP packet: sender={}",
                      vtnManager.getContainerName(),
                      HexEncode.bytesToHexStringFormat(spa));
            return PacketResult.IGNORED;
        }

        InetAddress tgtIp;
        try {
            tgtIp = InetAddress.getByAddress(arp.getTargetProtocolAddress());
        } catch (UnknownHostException e) {
            byte[] tpa = arp.getTargetProtocolAddress();
            LOG.error("{}: Ignore broken ARP packet: target={}",
                      vtnManager.getContainerName(),
                      HexEncode.bytesToHexStringFormat(tpa));
            return PacketResult.IGNORED;
        }

        short vlan = pctx.getVlan();
        Subnet subnet = getSubnet(sdrIp, vlan);
        if (subnet == null) {
            return PacketResult.IGNORED;
        }

        NodeConnector nc = pctx.getIncomingNodeConnector();
        if (!subnet.hasNodeConnector(nc)) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: Node connector does not match: subnet={}, " +
                          "incoming={}", vtnManager.getContainerName(),
                          subnet, nc);
            }
            return PacketResult.IGNORED;
        }

        if (LOG.isTraceEnabled()) {
            LOG.trace("{}: Subnet found: subnet={}, incoming={}, sender={}",
                      vtnManager.getContainerName(), subnet, nc, sdrIp);
        }

        HostNodeConnector requestor = null;
        byte[] src = pctx.getSourceAddress();
        if (NetUtils.isUnicastMACAddr(src)) {
            try {
                requestor = new HostNodeConnector(src, sdrIp, nc, vlan);
            } catch (ConstructionException e) {
                if (LOG.isErrorEnabled()) {
                    StringBuilder builder =
                        new StringBuilder(vtnManager.getContainerName());
                    builder.append(": Unable to create host: src=").
                        append(HexEncode.bytesToHexStringFormat(src)).
                        append(", sender=").append(sdrIp.getHostAddress()).
                        append(", incoming=").append(nc.toString()).
                        append(", vlan=").append((int)vlan);
                    LOG.error(builder.toString(), e);
                }
                return PacketResult.IGNORED;
            }

            // Notify the host tracker of new host entry.
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: Notify new host: ipaddr={}, host={}",
                          vtnManager.getContainerName(),
                          sdrIp.getHostAddress(), requestor);
            }
            vtnManager.notifyHost(requestor);
        }

        if (sdrIp.equals(tgtIp) || op == ARP.REPLY) {
            // In the case of gratuitous ARP request or ARP reply,
            // send ARP reply if there are hosts waiting for the ARP reply for
            // the sender IP address.
            return respondArpRequestors(src, sdrIp, vlan);
        }

        byte[] dst = pctx.getDestinationAddress();
        if (tgtIp.equals(subnet.getNetworkAddress())) {
            ISwitchManager swMgr = vtnManager.getSwitchManager();
            byte[] ctlrMac = swMgr.getControllerMAC();
            if (NetUtils.isBroadcastMACAddr(dst) ||
                Arrays.equals(dst, ctlrMac)) {
                // Send ARP reply if target is gateway IP.
                byte[] sha = arp.getSenderHardwareAddress();
                sendArp(nc, ARP.REPLY, ctlrMac, sha, tgtIp.getAddress(), spa,
                        vlan);
                return PacketResult.KEEP_PROCESSING;
            }
        }

        // Initiate ARP request.
        IfIptoHost ht = vtnManager.getHostTracker();
        HostNodeConnector host = ht.hostQuery(tgtIp);
        if (host == null) {
            // Add the requestor to the set of ongoing ARP requests so that
            // we can relay the reply when the target host responds.
            Requestor newrq = new Requestor(requestor);
            ObjectPair<InetAddress, Short> pair =
                new ObjectPair<InetAddress, Short>(tgtIp, vlan);
            synchronized (this) {
                Requestor rq = arpRequestors.putIfAbsent(pair, newrq);
                if (rq != null) {
                    rq.add(requestor);
                }
            }
            floodArpRequest(tgtIp, subnet);
            return PacketResult.KEEP_PROCESSING;
        }

        byte[] dladdr = host.getDataLayerAddressBytes();
        if (NetUtils.isBroadcastMACAddr(dst) || Arrays.equals(dladdr, dst)) {
            // Relay ARP reply from the known host.
            InetAddress haddr = host.getNetworkAddress();
            byte[] sha = arp.getSenderHardwareAddress();
            sendArp(nc, ARP.REPLY, dladdr, sha, haddr.getAddress(), spa, vlan);
            return PacketResult.KEEP_PROCESSING;
        }

        return PacketResult.IGNORED;
    }

    /**
     * Handle received IPv4 packet.
     *
     * @param pctx  The context of the received packet.
     * @param ipv4  IPv4 packet in the received packet.
     * @return  An indication if the packet should still be processed or
     *          we should stop it.
     */
    private PacketResult handlePacket(PacketContext pctx, IPv4 ipv4) {
        byte[] dst = NetUtils.intToByteArray4(ipv4.getDestinationAddress());
        InetAddress dstIp;
        try {
            dstIp = InetAddress.getByAddress(dst);
        } catch (UnknownHostException e) {
            LOG.error("{}: Ignore broken IPv4 packet: dst={}",
                      vtnManager.getContainerName(),
                      HexEncode.bytesToHexStringFormat(dst));
            return PacketResult.IGNORED;
        }

        short vlan = pctx.getVlan();
        Subnet subnet = getSubnet(dstIp, vlan);
        if (subnet == null) {
            return PacketResult.IGNORED;
        }

        // Send broadcast ARP request to discover host.
        floodArpRequest(dstIp, subnet);
        return PacketResult.KEEP_PROCESSING;
    }

    /**
     * Send an ARP message.
     *
     * @param outgoing  Outgoing node connector.
     * @param op        Operation code defined by ARP.
     * @param src       Source MAC address.
     * @param dst       Destination MAC address.
     * @param sender    Sender IP address.
     * @param target    Target IP address.
     * @param vlan      VLAN ID. Zero means VLAN tag should not be added.
     */
    private void sendArp(NodeConnector outgoing, short op, byte[] src,
                         byte[] dst, byte[] sender, byte[] target,
                         short vlan) {
        Ethernet ether = vtnManager.
            createArp(op, src, dst, sender, target, vlan);
        vtnManager.transmit(outgoing, ether);
    }

    /**
     * Flood an ARP broadcast request.
     *
     * @param target   Target IP address.
     * @param subnet   A subnet to which the target belongs.
     */
    private void floodArpRequest(InetAddress target, Subnet subnet) {
        Set<NodeConnector> ncSet = new HashSet<NodeConnector>();
        if (subnet.isFlatLayer2()) {
            vtnManager.collectUpEdgePorts(ncSet);
        } else {
            for (NodeConnector nc: subnet.getNodeConnectors()) {
                if (vtnManager.isEnabled(nc) && vtnManager.isEdgePort(nc)) {
                    ncSet.add(nc);
                }
            }
        }

        ISwitchManager swMgr = vtnManager.getSwitchManager();
        byte[] src = swMgr.getControllerMAC();
        byte[] dst = {-1, -1, -1, -1, -1, -1};
        byte[] spa = subnet.getNetworkAddress().getAddress();
        byte[] tpa = target.getAddress();
        short vlan = subnet.getVlan();

        for (NodeConnector nc: ncSet) {
            sendArp(nc, ARP.REQUEST, src, dst, spa, tpa, vlan);
        }
    }

    /**
     * Send an ARP reply to hosts waiting for the ARP reply.
     *
     * @param sha   MAC address to be set as sender hardware address.
     * @param spa   IP address to be set as sender protocol address.
     * @param vlan  VLAN ID of the network to which the sender belongs.
     * @return  An indication if the packet should still be processed or
     *          we should stop it.
     */
    private PacketResult respondArpRequestors(byte[] sha, InetAddress spa,
                                              short vlan) {
        Requestor rq;
        ObjectPair<InetAddress, Short> pair =
            new ObjectPair<InetAddress, Short>(spa, vlan);
        synchronized (this) {
            rq = arpRequestors.remove(pair);
        }

        if (rq == null) {
            return PacketResult.IGNORED;
        }

        for (HostNodeConnector host: rq.getHosts()) {
            byte[] tha = host.getDataLayerAddressBytes();
            InetAddress tpa = host.getNetworkAddress();
            NodeConnector port = host.getnodeConnector();

            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: Sending ARP Reply: " +
                          "sender={}/{}, target={}/{}, vlan={}",
                          vtnManager.getContainerName(),
                          HexEncode.bytesToHexStringFormat(sha), spa,
                          HexEncode.bytesToHexStringFormat(tha), tpa, vlan);
            }
            sendArp(port, ARP.REPLY, sha, tha, spa.getAddress(),
                    tpa.getAddress(), vlan);
        }

        return PacketResult.KEEP_PROCESSING;
    }

    /**
     * Scan ongoing ARP requests, and detect expired requests.
     */
    private void expireRequests() {
        for (Iterator<Map.Entry<ObjectPair<InetAddress, Short>, Requestor>>
                 it = arpRequestors.entrySet().iterator(); it.hasNext();) {
            Map.Entry<ObjectPair<InetAddress, Short>, Requestor> entry =
                it.next();
            Requestor req = entry.getValue();
            if (req.decrementTimer()) {
                // ARP request has expired.
                synchronized (this) {
                    it.remove();
                }
                if (LOG.isDebugEnabled()) {
                    ObjectPair<InetAddress, Short> pair = entry.getKey();
                    InetAddress addr = pair.getLeft();
                    Short vlan = pair.getRight();
                    LOG.debug("{}: ARP request timeout: addr={}, vlan={}",
                              vtnManager.getContainerName(), addr, vlan);
                }
            }
        }
    }

    /**
     * Search for a subnet by a pair of IP address and VLAN ID.
     *
     * @param addr  An IP address.
     * @param vlan  VLAN ID.
     * @return  A subnet object if found. {@code null} if not found.
     */
    private Subnet getSubnet(InetAddress addr, short vlan) {
        ISwitchManager swMgr = vtnManager.getSwitchManager();
        Subnet subnet = swMgr.getSubnetByNetworkAddress(addr);
        if (subnet == null) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: Subnet not found: addr={}",
                          vtnManager.getContainerName(), addr);
            }
        } else if (subnet.getVlan() != vlan) {
            if (LOG.isDebugEnabled()) {
                LOG.debug("{}: VLAN ID does not match: subnet={}, vlan={}",
                          vtnManager.getContainerName(), subnet, vlan);
            }
            subnet = null;
        }

        return subnet;
    }
}
