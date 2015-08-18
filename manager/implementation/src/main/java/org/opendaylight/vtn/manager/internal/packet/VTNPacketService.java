/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import java.util.Iterator;
import java.util.TimerTask;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentMap;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.Future;
import java.util.concurrent.atomic.AtomicReference;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.inventory.VTNInventoryListener;
import org.opendaylight.vtn.manager.internal.inventory.VtnNodeEvent;
import org.opendaylight.vtn.manager.internal.inventory.VtnPortEvent;
import org.opendaylight.vtn.manager.internal.util.SalNotificationListener;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorCallback;

import org.opendaylight.controller.sal.binding.api.NotificationService;

import org.opendaylight.yangtools.yang.common.RpcResult;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.PacketProcessingListener;
import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.PacketProcessingService;
import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.PacketReceived;
import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.TransmitPacketInputBuilder;

import org.opendaylight.controller.sal.packet.Packet;
import org.opendaylight.controller.sal.utils.EtherTypes;

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
     * Keep nodes which are not in service yet.
     *
     * <p>
     *   If a node is contained in this map, any packet from the node is
     *   ignored, and no packet is sent to the node.
     * </p>
     */
    private ConcurrentMap<SalNode, TimerTask>  disabledNodes =
        new ConcurrentHashMap<SalNode, TimerTask>();

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
     * @param packet  A {@link Packet} instance to be transmitted.
     */
    public void transmit(SalPort egress, Packet packet) {
        PacketProcessingService pps = packetService.get();
        if (pps == null) {
            return;
        }

        byte[] payload;
        try {
            payload = packet.serialize();
        } catch (Exception e) {
            LOG.error("Failed to transmit packet via " + egress, e);
            return;
        }

        TransmitPacketInputBuilder builder = new TransmitPacketInputBuilder();
        builder.setNode(egress.getNodeRef()).
            setEgress(egress.getNodeConnectorRef()).
            setPayload(payload);

        Future<RpcResult<Void>> f = pps.transmitPacket(builder.build());
        RpcErrorCallback<Void> cb = new RpcErrorCallback<>(
            LOG, "transmit-packet", "Failed to transmit packet.");
        vtnProvider.setCallback(f, cb);
    }

    /**
     * Add the given node to {@link #disabledNodes}.
     *
     * <p>
     *   Neighbor node discovery may not be completed on a newly detected node.
     *   If a PACKET_OUT is sent to a port in a newly detected node, it may
     *   cause broadcast packet storm because the controller can not
     *   determine internal port in a node yet.
     * </p>
     * <p>
     *   That is why we should disable any packet service on a newly detected
     *   node for a while.
     * </p>
     *
     * @param snode  A newly detected node.
     */
    private void addDisabledNode(final SalNode snode) {
        int edgeWait = vtnProvider.getVTNConfig().getNodeEdgeWait();
        if (edgeWait <= 0) {
            return;
        }

        if (packetService.get() == null) {
            // Packet service is already closed.
            return;
        }

        // Create a timer task to remove the node from disabledNodes.
        TimerTask task = new TimerTask() {
            @Override
            public void run() {
                if (disabledNodes.remove(snode) != null) {
                    LOG.info("{}: Start packet service", snode);
                }
            }
        };

        if (disabledNodes.putIfAbsent(snode, task) == null) {
            vtnProvider.getTimer().schedule(task, edgeWait);
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

    // AutoCloseable

    /**
     * Close the internal packet service.
     */
    @Override
    public void close() {
        listeners.clear();
        super.close();
        packetService.set(null);

        for (Iterator<TimerTask> it = disabledNodes.values().iterator();
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

        SalNode snode = ingress.getSalNode();
        if (disabledNodes.containsKey(snode)) {
            LOG.trace("Ignore packet from disabled node: {}", snode);
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
     * {@inheritDoc}
     */
    @Override
    public void notifyVtnNode(VtnNodeEvent ev) {
        switch (ev.getUpdateType()) {
        case CREATED:
            addDisabledNode(ev.getSalNode());
            break;

        case REMOVED:
            TimerTask task = disabledNodes.get(ev.getSalNode());
            if (task != null) {
                task.cancel();
            }
            break;

        default:
            break;
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void notifyVtnPort(VtnPortEvent ev) {
        // Nothing to do.
    }
}
