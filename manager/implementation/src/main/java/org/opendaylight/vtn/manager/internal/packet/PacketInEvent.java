/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.packet;

import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.tx.TxEvent;

import org.opendaylight.controller.sal.core.ConstructionException;
import org.opendaylight.controller.sal.packet.Ethernet;
import org.opendaylight.controller.sal.packet.PacketException;
import org.opendaylight.controller.sal.packet.RawPacket;

import org.opendaylight.yang.gen.v1.urn.opendaylight.packet.service.rev130709.PacketReceived;

/**
 * {@code PacketInEvent} describes an event which notifies a received packet
 * via PACKET_IN message.
 */
public final class PacketInEvent extends TxEvent {
    /**
     * The target packet listener.
     */
    private final VTNPacketListener  listener;

    /**
     * A {@link SalPort} instance which specifies the ingress switch port.
     */
    private final SalPort  ingressPort;

    /**
     * A {@link RawPacket} instance which represents the received packet.
     */
    private final RawPacket  payload;

    /**
     * Decoded ethernet frame.
     */
    private final Ethernet  ethernet;

    /**
     * Construct a new instance.
     *
     * @param l        A {@link VTNPacketListener} instance.
     * @param rcv      A {@link PacketReceived} instance notified by MD-SAL.
     * @param ingress  A {@link SalPort} instance which indicates the
     *                 ingress switch port.
     * @throws ConstructionException
     *    Received packet is broken.
     * @throws PacketException
     *    Received packet is broken.
     * @throws IllegalArgumentException
     *    The given packet is not supprted.
     */
    public PacketInEvent(VTNPacketListener l, PacketReceived rcv,
                         SalPort ingress)
        throws ConstructionException, PacketException {
        listener = l;
        ingressPort = ingress;

        byte[] bytes = rcv.getPayload();
        payload = new RawPacket(bytes);
        payload.setIncomingNodeConnector(ingressPort.getAdNodeConnector());
        ethernet = new Ethernet();
        ethernet.deserialize(bytes, 0, bytes.length * Byte.SIZE);
    }

    /**
     * Create a copy of the given event for the given listener.
     *
     * @param l   A {@link VTNPacketListener} instance.
     * @param ev  A {@link PacketInEvent} instance.
     */
    public PacketInEvent(VTNPacketListener l, PacketInEvent ev) {
        listener = l;
        ingressPort = ev.ingressPort;
        payload = ev.payload;
        ethernet = ev.ethernet;
    }

    /**
     * Return a {@link SalPort} instance which specifies the ingress switch
     * port.
     *
     * @return  A {@link SalPort} instance.
     */
    public SalPort getIngressPort() {
        return ingressPort;
    }

    /**
     * Return a {@link RawPacket} instance which represents the received
     * packet.
     *
     * @return  A {@link RawPacket} instance.
     */
    public RawPacket getPayload() {
        return payload;
    }

    /**
     * Return a {@link Ethernet} instance which represents the received
     * packet.
     *
     * @return  A {@link Ethernet} instance.
     */
    public Ethernet getEthernet() {
        return ethernet;
    }

    // TxEvent

    /**
     * {@inheritDoc}
     */
    @Override
    protected void notifyEvent() throws VTNException {
        listener.notifyPacket(this);
    }
}
