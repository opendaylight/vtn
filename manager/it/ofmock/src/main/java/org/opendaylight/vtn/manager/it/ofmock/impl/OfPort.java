/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock.impl;

import java.util.Deque;
import java.util.LinkedList;
import java.util.Objects;
import java.util.Random;
import java.util.concurrent.TimeoutException;

import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.VtnOpenflowVersion;

import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnectorUpdated;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.inventory.rev130819.FlowCapableNodeConnectorUpdatedBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.topology.discovery.rev130819.LinkDiscoveredBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.topology.discovery.rev130819.LinkRemovedBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortFeatures;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.PortNumberUni;
import org.opendaylight.yang.gen.v1.urn.opendaylight.flow.types.port.rev130925.flow.capable.port.StateBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorRemoved;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorRemovedBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorUpdated;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorUpdatedBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code OfPort} describes a physical port of OpenFlow switch.
 */
public final class OfPort {
    /**
     * The number of bytes in a MAC address.
     */
    private static final int  MAC_ADDRESS_SIZE = 6;

    /**
     * The multicast bit in the first octet of a MAC address.
     */
    private static final int  MAC_MULTICAST = 0x01;

    /**
     * The mask value for a byte value.
     */
    private static final int  MASK_BYTE = 0xff;

    /**
     * Pseudo random number generator.
     */
    private static final Random  RANDOM = new Random();

    /**
     * An empty port feature.
     */
    private static final PortFeatures  FEATURES_EMPTY =
        new PortFeatures(Boolean.FALSE, Boolean.FALSE, Boolean.FALSE,
                         Boolean.FALSE, Boolean.FALSE, Boolean.FALSE,
                         Boolean.FALSE, Boolean.FALSE, Boolean.FALSE,
                         Boolean.FALSE, Boolean.FALSE, Boolean.FALSE,
                         Boolean.FALSE, Boolean.FALSE, Boolean.FALSE,
                         Boolean.FALSE);

    /**
     * Current feature (copper, one-gb-hd).
     */
    private static final PortFeatures  FEATURES_CURRENT =
        new PortFeatures(Boolean.FALSE, Boolean.TRUE, Boolean.FALSE,
                         Boolean.FALSE, Boolean.FALSE, Boolean.FALSE,
                         Boolean.FALSE, Boolean.FALSE, Boolean.TRUE,
                         Boolean.FALSE, Boolean.FALSE, Boolean.FALSE,
                         Boolean.FALSE, Boolean.FALSE, Boolean.FALSE,
                         Boolean.FALSE);

    /**
     * Port configuration.
     */
    private static final PortConfig  PORT_CONFIG =
        new PortConfig(Boolean.FALSE, Boolean.FALSE, Boolean.FALSE,
                       Boolean.FALSE);

    /**
     * Link speed in kbps.
     */
    private static final Long  SPEED_KBPS = Long.valueOf(1000000L);

    /**
     * OpenFlow protocol version.
     */
    private final VtnOpenflowVersion  ofVersion;

    /**
     * The port number.
     */
    private long  portNumber;

    /**
     * The name of this port.
     */
    private String  portName;

    /**
     * Link state of this port.
     */
    private boolean  linkUp;

    /**
     * MAC address of the port.
     */
    private final MacAddress  macAddress;

    /**
     * The identifier string of this port.
     */
    private final String  portIdentifier;

    /**
     * The instance identifier of this port.
     */
    private final InstanceIdentifier<NodeConnector>  portPath;

    /**
     * The identifier string of the peer port.
     */
    private String  peerIdentifier;

    /**
     * A list of transmitted packets.
     */
    private final Deque<byte[]>  transmittedPackets = new LinkedList<>();

    /**
     * Construct a new instance.
     *
     * @param ver  OpenFlow protocol version.
     * @param nid  The node identifier.
     * @param id   The port number.
     */
    public OfPort(VtnOpenflowVersion ver, String nid, long id) {
        ofVersion = ver;
        portNumber = id;
        portIdentifier = nid + ":" + id;
        portName = "eth" + id;
        macAddress = createMacAddress();
        portPath = OfMockUtils.getPortPath(portIdentifier);
    }

    /**
     * Return the name of this port.
     *
     * @return  The port name.
     */
    public String getPortName() {
        return portName;
    }

    /**
     * Return the identifier string of the port.
     *
     * @return  The identifier string of the port.
     */
    public String getPortIdentifier() {
        return portIdentifier;
    }

    /**
     * Return the path to this port.
     *
     * @return  An {@link InstanceIdentifier} instance.
     */
    public InstanceIdentifier<NodeConnector> getPortPath() {
        return portPath;
    }

    /**
     * Return a {@link NodeConnectorUpdated} instance which indicates this port
     * has been updated.
     *
     * @return  A {@link NodeConnectorUpdated} instance.
     */
    public NodeConnectorUpdated getNodeConnectorUpdated() {
        NodeConnectorUpdatedBuilder builder = new NodeConnectorUpdatedBuilder().
            setId(new NodeConnectorId(portIdentifier)).
            setNodeConnectorRef(new NodeConnectorRef(portPath));

        boolean down = Boolean.valueOf(!linkUp);
        StateBuilder stBuilder = new StateBuilder().setBlocked(Boolean.FALSE).
            setLinkDown(down).setLive(Boolean.FALSE);

        FlowCapableNodeConnectorUpdatedBuilder fcBuilder =
            new FlowCapableNodeConnectorUpdatedBuilder().
            setState(stBuilder.build()).setName(portName).
            setHardwareAddress(macAddress).
            setAdvertisedFeatures(FEATURES_EMPTY).
            setPeerFeatures(FEATURES_EMPTY).setSupported(FEATURES_CURRENT).
            setCurrentFeature(FEATURES_CURRENT).setConfiguration(PORT_CONFIG).
            setPortNumber(new PortNumberUni(Long.valueOf(portNumber)));

        if (ofVersion != VtnOpenflowVersion.OF10) {
            fcBuilder.setCurrentSpeed(SPEED_KBPS).
                setMaximumSpeed(SPEED_KBPS);
        }

        builder.addAugmentation(FlowCapableNodeConnectorUpdated.class,
                                fcBuilder.build());

        return builder.build();
    }

    /**
     * Return a {@link NodeConnectorRemoved} instance which indicates this port
     * has been removed.
     *
     * @return  A {@link NodeConnectorRemoved} instance.
     */
    public NodeConnectorRemoved getNodeConnectorRemoved() {
        NodeConnectorRemovedBuilder builder = new NodeConnectorRemovedBuilder().
            setNodeConnectorRef(new NodeConnectorRef(portPath));
        return builder.build();
    }

    /**
     * Get the identifier of the peer port.
     *
     * @return  The identifier of the peer port if this port is connected to
     *          another switch port.
     *          {@code null} if not connected.
     */
    public String getPeerIdentifier() {
        return peerIdentifier;
    }

    /**
     * Set the identifier of the peer port.
     *
     * @param provider  The ofmock provider service.
     * @param peer      The identifier of the peer port.
     * @return  {@code true} only if link state notification has been
     *          actually published.
     */
    public boolean setPeerIdentifier(OfMockProvider provider, String peer) {
        if (Objects.equals(peerIdentifier, peer)) {
            return false;
        }

        String oldPeer = peerIdentifier;
        peerIdentifier = peer;
        if (linkUp) {
            if (oldPeer != null) {
                notifyLinkRemoved(provider, oldPeer);
            }
            if (peer != null) {
                notifyLinkDiscovered(provider, peer);
            }
        }

        return linkUp;
    }

    /**
     * Get the link state of this port.
     *
     * @return  {@code true} if this port is in up state.
     *          {@code false} if this port is in down state.
     */
    public boolean isUp() {
        return linkUp;
    }

    /**
     * Set the link state of this port.
     *
     * @param provider  The ofmock provider service.
     * @param state     New link state.
     * @return  {@code true} if the port state has been changed.
     *          {@code false} not changed.
     */
    public boolean setPortState(OfMockProvider provider, boolean state) {
        boolean changed = (linkUp != state);

        if (changed) {
            linkUp = state;
            provider.publish(getNodeConnectorUpdated());

            String peer = peerIdentifier;
            if (peer != null) {
                if (state) {
                    notifyLinkDiscovered(provider, peer);
                } else {
                    notifyLinkRemoved(provider, peer);
                }
            }
        }

        return changed;
    }

    /**
     * Transmit the given packet to this port.
     *
     * @param payload  A byte array which represents the packet.
     */
    public synchronized void transmit(byte[] payload) {
        transmittedPackets.addLast(payload);
        notifyAll();
    }

    /**
     * Dequeue a packet from the packet transmission queue.
     *
     * @return  A byte array which represents the packet if at least one packet
     *          is present in the packet transmission queue.
     *          {@code null} if no packet is present.
     */
    public synchronized byte[] getTransmittedPacket() {
        return (transmittedPackets.isEmpty())
            ? null
            : transmittedPackets.removeFirst();
    }

    /**
     * Wait for a packet to be transmitted.
     *
     * @param timeout  The number of milliseconds to wait.
     * @return  A byte array which represents the packet.
     *          {@code null} if no packet is present on the transmission queue.
     * @throws InterruptedException
     *    The calling thread was interrupted.
     * @throws TimeoutException
     *    {@code timeout} is zero or negative.
     */
    public synchronized byte[] awaitTransmittedPacket(long timeout)
        throws InterruptedException, TimeoutException {
        byte[] packet = getTransmittedPacket();
        if (packet == null) {
            if (timeout <= 0) {
                throw new TimeoutException();
            }
            wait(timeout);
        }

        return packet;
    }

    /**
     * Clear the packet transmission queue.
     *
     * <p>
     *   This method must be called with holding the global lock.
     * </p>
     */
    public synchronized void clearTransmittedPacket() {
        transmittedPackets.clear();
    }

    /**
     * Publish notification that notifies this switch port.
     *
     * @param provider  The ofmock provider service.
     */
    public void publish(OfMockProvider provider) {
        provider.publish(getNodeConnectorUpdated());
    }

    /**
     * Publish notification that notifies the inter-switch link.
     *
     * @param provider  The ofmock provider service.
     */
    public void publishLink(OfMockProvider provider) {
        notifyLinkDiscovered(provider, peerIdentifier);
    }

    /**
     * Create a MAC address using pseudo random number generator.
     *
     * @return  A {@link MacAddress} instance.
     */
    private MacAddress createMacAddress() {
        byte[] mac = new byte[MAC_ADDRESS_SIZE];
        RANDOM.nextBytes(mac);
        String sep = "";
        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < mac.length; i++) {
            int b = (mac[i] & MASK_BYTE);
            if (i == 0) {
                b &= ~MAC_MULTICAST;
            }

            builder.append(sep).append(String.format("%02x", b));
            sep = ":";
        }

        return new MacAddress(builder.toString());
    }

    /**
     * Send notification that indicates an inter-switch link has been
     * discovered.
     *
     * @param provider  The ofmock provider service.
     * @param peer      The identifier of the peer port.
     */
    private void notifyLinkDiscovered(OfMockProvider provider, String peer) {
        LinkDiscoveredBuilder builder = new LinkDiscoveredBuilder();
        NodeConnectorRef sref = new NodeConnectorRef(portPath);
        InstanceIdentifier<NodeConnector> dpath =
            OfMockUtils.getPortPath(peer);
        NodeConnectorRef dref = new NodeConnectorRef(dpath);
        builder.setSource(sref).setDestination(dref);
        provider.publish(builder.build());
    }

    /**
     * Send notification that indicates an inter-switch link has been
     * removed.
     *
     * @param provider  The ofmock provider service.
     * @param peer      The identifier of the peer port.
     */
    private void notifyLinkRemoved(OfMockProvider provider, String peer) {
        LinkRemovedBuilder builder = new LinkRemovedBuilder();
        NodeConnectorRef sref = new NodeConnectorRef(portPath);
        InstanceIdentifier<NodeConnector> dpath =
            OfMockUtils.getPortPath(peer);
        NodeConnectorRef dref = new NodeConnectorRef(dpath);
        builder.setSource(sref).setDestination(dref);
        provider.publish(builder.build());
    }
}
