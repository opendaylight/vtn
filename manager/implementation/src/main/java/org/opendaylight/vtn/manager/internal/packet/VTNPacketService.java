/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.apache.commons.lang3.tuple.Pair;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.packet.Packet;
import org.opendaylight.vtn.manager.util.EtherTypes;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.inventory.VTNInventoryListener;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.util.SalNotificationListener;
import org.opendaylight.vtn.manager.internal.util.inventory.InventoryReader;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorCallback;

import org.opendaylight.controller.md.sal.binding.api.NotificationService;

import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnUpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.PacketProcessingListener;
import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.PacketProcessingService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.PacketReceived;
import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.TransmitPacketInputBuilder;

/**
 * Provider of internal packet services.
 */
public final class VTNPacketService extends SalNotificationListener
    implements PacketProcessingListener, VTNInventoryListener {
    /**
     * Logger instance.
     */
    private static final Logger  LOG =
        LoggerFactory.getLogger(VTNPacketService.class);

    /**
     * VTN Manager provider service.
     */
    private final VTNManagerProvider  vtnProvider;

    /**
     * MD-SAL packet processing service.
     */
    private final AtomicReference<PacketProcessingService>  packetService =
        new AtomicReference<PacketProcessingService>();

    /**
     * A list of VTN packet listeners.
     */
    private final CopyOnWriteArrayList<VTNPacketListener>  listeners =
        new CopyOnWriteArrayList<VTNPacketListener>();

    /**
     * A set of physical switch ports waiting for completion of topology
     * detection.
     *
     * <p>
     *   If a port is contained in this map, any packet from that port is
     *   ignored, and no packet is sent to that port.
     * </p>
     */
    private ConcurrentMap<SalPort, TimerTask>  topologyWaiting =
        new ConcurrentHashMap<SalPort, TimerTask>();

    /**
     * Construct a new instance.
     *
     * @param provider  A VTN Manager provider service.
     * @param nsv       A {@link NotificationService} service instance.
     */
    public VTNPacketService(VTNManagerProvider provider,
                            NotificationService nsv) {
        vtnProvider = provider;

        try {
            // Get MD-SAL packet processing service.
            PacketProcessingService pps = provider.
                getRpcService(PacketProcessingService.class);
            packetService.set(pps);

            // Register MD-SAL notification listener.
            registerListener(nsv);
        } catch (Exception e) {
            String msg = "Failed to initialize VTN packet service.";
            LOG.error(msg, e);
            close();
            throw new IllegalStateException(msg, e);
        }
    }

    /**
     * Add the given VTN packet listener.
     *
     * @param l  A VTN packet listener.
     */
    public void addListener(VTNPacketListener l) {
        listeners.addIfAbsent(l);
    }

    /**
     * Transmit the given packet.
     *
     * @param egress  A {@link SalPort} instance which specifies the egress
     *                switch port.
     * @param packet  A {@link Packet} instance to transmit.
     */
    public void transmit(SalPort egress, Packet packet) {
        PacketProcessingService pps = packetService.get();
        if (pps != null) {
            transmit(pps, egress, packet);
        }
    }

    /**
     * Transmit all the given packets.
     *
     * @param packets  A list of packets to transmit.
     *                 The left of each elements indicates the egress switch
     *                 port, and the right indiciates a packet to transmit.
     */
    public void transmit(List<Pair<SalPort, Packet>> packets) {
        PacketProcessingService pps = packetService.get();
        if (pps != null) {
            for (Pair<SalPort, Packet> pkt: packets) {
                transmit(pps, pkt.getLeft(), pkt.getRight());
            }
        }
    }

    /**
     * Transmit the given packet.
     *
     * @param pps     A packet-procssing service.
     * @param egress  A {@link SalPort} instance which specifies the egress
     *                switch port.
     * @param packet  A {@link Packet} instance to be transmitted.
     */
    private void transmit(PacketProcessingService pps, SalPort egress,
                          Packet packet) {
        if (topologyWaiting.containsKey(egress)) {
            LOG.trace("Ignore packet transmission due to topology " +
                      "detection: {}", egress);
        } else {
            byte[] payload;
            try {
                payload = packet.serialize();
            } catch (Exception e) {
                LOG.error("Failed to transmit packet via " + egress, e);
                return;
            }

            TransmitPacketInputBuilder builder =
                new TransmitPacketInputBuilder().
                setNode(egress.getNodeRef()).
                setEgress(egress.getNodeConnectorRef()).
                setPayload(payload);

            Future<RpcResult<Void>> f = pps.transmitPacket(builder.build());
            RpcErrorCallback<Void> cb = new RpcErrorCallback<>(
                LOG, "transmit-packet", "Failed to transmit packet.");
            vtnProvider.setCallback(f, cb);
        }
    }

    /**
     * Add the given port to {@link #topologyWaiting}.
     *
     * <p>
     *   Inter-switch link detection may be still running on a port when that
     *   port status has been changed to UP state. If a PACKET_OUT is sent to
     *   that port, it may cause broadcast packet storm because the controller
     *   can not determine ISL port yet.
     * </p>
     * <p>
     *   That is why we should disable any packet service on a newly enabled
     *   port for a while.
     * </p>
     *
     * @param sport  A newly enabled port.
     */
    private void addTopologyWaiting(final SalPort sport) {
        int topoWait = vtnProvider.getVTNConfig().getTopologyWait();
        if (topoWait <= 0) {
            // Topology wait is disabled.
            return;
        }

        // Create a timer task to remove the port from topologyWaiting.
        TimerTask task = new TimerTask() {
            @Override
            public void run() {
                if (topologyWaiting.remove(sport) != null) {
                    LOG.info("{}: Start packet service.", sport);
                }
            }
        };

        if (topologyWaiting.putIfAbsent(sport, task) == null) {
            if (packetService.get() == null) {
                // Packet service is already closed.
                topologyWaiting.remove(sport);
            } else {
                LOG.debug("{}: Wait for completion of topology detection.",
                          sport);
                vtnProvider.getTimer().schedule(task, topoWait);
            }
        }
    }

    /**
     * Construct a new PACKET_IN event.
     *
     * @param listener      A PACKET_IN event listener.
     * @param notification  A PACKET_IN notification.
     * @param ingress       A {@link SalPort} instance which specifies the
     *                      ingress switch port.
     * @return  A {@link PacketInEvent} instance if the given notification
     *          should be delivered to listeners.
     *          {@code null} if the given notification should be ignored.
     */
    private PacketInEvent newPacketInEvent(
        VTNPacketListener listener, PacketReceived notification,
        SalPort ingress) {
        PacketInEvent ev;
        try {
            ev = new PacketInEvent(listener, notification, ingress);
        } catch (IllegalArgumentException e) {
            LOG.debug("Ignore received packet.", e);
            return null;
        } catch (Exception e) {
            LOG.error("Failed to create PacketInEvent: " + notification, e);
            return null;
        }

        if (ev.getEthernet().getEtherType() == EtherTypes.LLDP.shortValue()) {
            // Ignore LLDP packet.
            return null;
        }

        return ev;
    }

    /**
     * Determine whether the topology wait on the port specified by the
     * given port event should terminate or not.
     *
     * @param ev  A {@link VtnPortEvent} instance which notifies the port
     *            change.
     * @return  A string which indicates the reason why the topology wait
     *          should terminate. {@code null} if the topology wait should not
     *          terminate.
     * @throws VTNException  An error occurred.
     */
    private String shouldTopologyWaitTerminate(VtnPortEvent ev)
        throws VTNException {
        if (ev.getUpdateType() == VtnUpdateType.REMOVED) {
            return "port has been removed";
        }

        if (Boolean.FALSE.equals(ev.getStateChange())) {
            return "port is down";
        }

        if (Boolean.TRUE.equals(ev.getInterSwitchLinkChange())) {
            return "inter-switch link has been detected";
        }

        InventoryReader reader = ev.getTxContext().
            getReadSpecific(InventoryReader.class);
        SalPort sport = ev.getSalPort();
        return (reader.isStaticEdgePort(sport))
            ? "static edge port" : null;
    }

    // AutoCloseable

    /**
     * Close the internal packet service.
     */
    @Override
    public void close() {
        listeners.clear();
        super.close();
        packetService.set(null);

        for (Iterator<TimerTask> it = topologyWaiting.values().iterator();
             it.hasNext();) {
            TimerTask task = it.next();
            task.cancel();
            it.remove();
        }
    }

    // PacketProcessingListener

    /**
     * Invoked when a PACKET_IN message has been notified.
     *
     * @param notification  A PACKET_IN notification.
     */
    @Override
    public void onPacketReceived(PacketReceived notification) {
        if (notification == null) {
            LOG.debug("Ignore null notification.");
            return;
        }

        NodeConnectorRef nref = notification.getIngress();
        SalPort ingress = SalPort.create(nref);
        if (ingress == null) {
            LOG.trace("Ignore packet received on unsupported port: {}", nref);
            return;
        }

        if (topologyWaiting.containsKey(ingress)) {
            LOG.trace("Ignore received packet due to topology detection: {}",
                      ingress);
            return;
        }

        Iterator<VTNPacketListener> it = listeners.iterator();
        if (it.hasNext()) {
            PacketInEvent ev =
                newPacketInEvent(it.next(), notification, ingress);
            if (ev != null) {
                vtnProvider.post(ev);
                while (it.hasNext()) {
                    vtnProvider.post(new PacketInEvent(it.next(), ev));
                }
            }
        }
    }

    // CloseableContainer

    /**
     * {@inheritDoc}
     */
    @Override
    protected Logger getLogger() {
        return LOG;
    }

    // VTNInventoryListener

    /**
     * Invoked when a node information has been added or removed.
     *
     * @param ev  A {@link VtnNodeEvent} instance.
     */
    @Override
    public void notifyVtnNode(VtnNodeEvent ev) {
        if (ev.getUpdateType() == VtnUpdateType.REMOVED) {
            // Terminate topology waits on ports contained by the removed node.
            long num = ev.getSalNode().getNodeNumber();
            Set<Map.Entry<SalPort, TimerTask>> eset =
                topologyWaiting.entrySet();
            for (Iterator<Map.Entry<SalPort, TimerTask>> it = eset.iterator();
                 it.hasNext();) {
                Map.Entry<SalPort, TimerTask> ent = it.next();
                SalPort sport = ent.getKey();
                if (sport.getNodeNumber() == num) {
                    LOG.debug("{}: Terminate topology wait on removed node.",
                              sport);
                    TimerTask task = ent.getValue();
                    task.cancel();
                }
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void notifyVtnPort(VtnPortEvent ev) throws VTNException {
        String reason = shouldTopologyWaitTerminate(ev);
        if (reason != null) {
            SalPort sport = ev.getSalPort();
            TimerTask task = topologyWaiting.remove(sport);
            if (task != null) {
                LOG.debug("{}: Terminate topology wait: reason={}",
                          sport, reason);
                task.cancel();
            }
        } else if (Boolean.TRUE.equals(ev.getStateChange())) {
            // Wait for completion of topology detection on this port.
            addTopologyWaiting(ev.getSalPort());
        }
    }
}
