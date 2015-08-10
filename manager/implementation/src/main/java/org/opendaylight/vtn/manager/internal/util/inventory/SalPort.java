/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import org.opendaylight.vtn.manager.util.NumberUtils;

import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;
import org.opendaylight.yangtools.yang.binding.InstanceIdentifier.InstanceIdentifierBuilder;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPort;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.node.info.VtnPortKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLink;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.inventory.rev150209.vtn.port.info.PortLinkKey;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnectorKey;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.LinkId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.TpId;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.Destination;
import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.link.attributes.Source;

/**
 * This class represents SAL node connector identifier corresponding to
 * physical port in OpenFlow switch.
 */
public final class SalPort extends SalNode {
    /**
     * Maximum value of physical switch port index (OF 1.3 and later).
     */
    private static final long  MAX_PORT = 0xffffff00L;

    /**
     * Port number.
     */
    private final long  portNumber;

    /**
     * Cache for AD-SAL node connector.
     *
     * <p>
     *   Note that this field does not affect object identity.
     * </p>
     */
    private org.opendaylight.controller.sal.core.NodeConnector  adNodeConnector;

    /**
     * Determine whether the given MD-SAL node connector ID indicates a
     * logical port of OpenFlow switch or not.
     *
     * @param id  A {@link NodeConnectorId} instance.
     * @return    {@code true} if the given ID indicates a logical port of
     *            OpenFlow switch. Otherwise {@code false}.
     */
    public static boolean isLogicalPort(NodeConnectorId id) {
        if (id != null) {
            try {
                String str = id.getValue();
                parse(str);
            } catch (NumberFormatException e) {
                return true;
            } catch (RuntimeException e) {
                // Unexpected format.
            }
        }

        return false;
    }

    /**
     * Convert a MD-SAL node connector ID into a {@code SalPort} instance.
     *
     * @param id  A {@link NodeConnectorId} instance.
     * @return  A {@code SalPort} instance on success.
     *          {@code null} on failure.
     */
    public static SalPort create(NodeConnectorId id) {
        if (id == null) {
            return null;
        }

        return create(id.getValue());
    }

    /**
     * Convert a MD-SAL node connector reference into a {@code SalPort}
     * instance.
     *
     * @param ref  A {@link NodeConnectorRef} instance.
     * @return  A {@code SalPort} instance on success.
     *          {@code null} on failure.
     */
    public static SalPort create(NodeConnectorRef ref) {
        if (ref == null) {
            return null;
        }

        InstanceIdentifier<?> id = ref.getValue();
        if (id == null) {
            return null;
        }

        NodeConnectorKey key =
            id.firstKeyOf(NodeConnector.class, NodeConnectorKey.class);
        if (key == null) {
            return null;
        }

        return create(key.getId());
    }

    /**
     * Convert a link termination point within source node into a
     * {@code SalPort} instance.
     *
     * @param src  A {@link Source} instance.
     * @return  A {@code SalPort} instance on success.
     *          {@code null} on failure.
     */
    public static SalPort create(Source src) {
        if (src == null) {
            return null;
        }

        return create(src.getSourceTp());
    }

    /**
     * Convert a link termination point within destination node into a
     * {@code SalPort} instance.
     *
     * @param dst  A {@link Destination} instance.
     * @return  A {@code SalPort} instance on success.
     *          {@code null} on failure.
     */
    public static SalPort create(Destination dst) {
        if (dst == null) {
            return null;
        }

        return create(dst.getDestTp());
    }

    /**
     * Convert a link termination point into a {@code SalPort}
     * instance.
     *
     * @param tpId  A {@link TpId} instance.
     * @return  A {@code SalPort} instance on success.
     *          {@code null} on failure.
     */
    public static SalPort create(TpId tpId) {
        if (tpId == null) {
            return null;
        }

        return create(tpId.getValue());
    }

    /**
     * Convert a string representation of MD-SAL node connector ID into a
     * {@code SalPort} instance.
     *
     * @param id  A string representation of MD-SAL node connector ID.
     * @return  A {@code SalPort} instance on success.
     *          {@code null} on failure.
     */
    public static SalPort create(String id) {
        try {
            return parse(id);
        } catch (Exception e) {
            return null;
        }
    }

    /**
     * Create a {@link SalPort} instance from the given pair of node and port
     * identifier.
     *
     * @param nodeId  A node identifier.
     * @param portId  A string representation of the port number.
     * @return  A {@code SalPort} instance on success.
     *          {@code null} on failure.
     */
    public static SalPort create(long nodeId, String portId) {
        try {
            long portNum = Long.decode(portId).longValue();
            if (portNum < 0 || portNum > MAX_PORT) {
                return null;
            }

            return new SalPort(nodeId, portNum);
        } catch (RuntimeException e) {
            return null;
        }
    }

    /**
     * Convert a AD-SAL node connector into a {@code SalPort} instance.
     *
     * @param nc  An AD-SAL node connector instance.
     * @return  A {@code SalPort} instance on success.
     *          {@code null} on failure.
     */
    public static SalPort create(
        org.opendaylight.controller.sal.core.NodeConnector nc) {
        if (nc == null || !NodeConnectorIDType.OPENFLOW.equals(nc.getType())) {
            return null;
        }

        SalNode snode = SalNode.create(nc.getNode());
        if (snode == null) {
            return null;
        }

        Object o = nc.getID();
        SalPort sport;
        if (o instanceof Short) {
            short s = ((Short)o).shortValue();
            long num = (s >= 0) ? s : (long)s & NumberUtils.MASK_SHORT;
            sport = new SalPort(snode.getNodeNumber(), num, nc);
        } else {
            sport = null;
        }

        return sport;
    }

    /**
     * Parse the given MD-SAL node connector ID.
     *
     * @param id  A MD-SAL node connector ID.
     * @return    A {@link SalPort} on success.
     *            {@code null} on failure.
     * @throws NumberFormatException
     *    The given node connector ID indicates a logical switch port.
     */
    private static SalPort parse(String id) {
        if (id == null) {
            return null;
        }

        int index = id.lastIndexOf(MD_ID_SEPARATOR);
        if (index <= 0) {
            return null;
        }

        String nodeId = id.substring(0, index);
        SalNode snode = SalNode.create(nodeId);
        if (snode == null) {
            return null;
        }

        Long l = Long.valueOf(id.substring(index + 1));
        long portNum = l.longValue();
        if (portNum < 0 || portNum > MAX_PORT) {
            return null;
        }

        return new SalPort(snode.getNodeNumber(), portNum, id);
    }

    /**
     * Construct a new instance.
     *
     * @param nodeId  A node identifier.
     * @param portId  A port identifier.
     */
    public SalPort(long nodeId, long portId) {
        super(nodeId);
        portNumber = portId;
    }

    /**
     * Construct a new instance with specifying a string representation of
     * this instance.
     *
     * @param nodeId  A node identifier.
     * @param portId  A port identifier.
     * @param str     A string representation of this instance.
     */
    public SalPort(long nodeId, long portId, String str) {
        super(nodeId, str);
        portNumber = portId;
    }

    /**
     * Construct a new instance with specifying an AD-SAL node connector
     * corresponding to this instance.
     *
     * @param nodeId  A node identifier.
     * @param portId  A port identifier.
     * @param nc      An AD-SAL node connector instance.
     */
    public SalPort(long nodeId, long portId,
                   org.opendaylight.controller.sal.core.NodeConnector nc) {
        super(nodeId, (nc == null) ? null : nc.getNode());
        portNumber = portId;
        adNodeConnector = nc;
    }

    /**
     * Return a port number.
     *
     * @return  A port number.
     */
    public long getPortNumber() {
        return portNumber;
    }

    /**
     * Return a new {@link SalNode} instance which represents the location of
     * the swtich to which this switch port belongs.
     *
     * @return  A {@link SalNode} instance.
     */
    public SalNode getSalNode() {
        return new SalNode(getNodeNumber());
    }

    /**
     * Return a {@link NodeConnectorId} instance corresponding to this
     * instance.
     *
     * @return  A {@link NodeConnectorId} instance.
     */
    public NodeConnectorId getNodeConnectorId() {
        return new NodeConnectorId(toString());
    }

    /**
     * Return a {@link NodeConnectorKey} instance corresponding to this
     * instance.
     *
     * @return  A {@link NodeConnectorKey} instance.
     */
    public NodeConnectorKey getNodeConnectorKey() {
        return new NodeConnectorKey(getNodeConnectorId());
    }

    /**
     * Return a {@link NodeConnectorRef} instance corresponding to this
     * instance.
     *
     * @return  An {@link NodeConnectorRef} instance.
     */
    public NodeConnectorRef getNodeConnectorRef() {
        return new NodeConnectorRef(getNodeConnectorIdentifier());
    }

    /**
     * Return an instance identifier that specifies MD-SAL node instance.
     *
     * @return  An {@link InstanceIdentifier} instance.
     */
    public InstanceIdentifier<NodeConnector> getNodeConnectorIdentifier() {
        return getNodeIdentifierBuilder().
            child(NodeConnector.class, getNodeConnectorKey()).build();
    }

    /**
     * Return a {@link VtnPortKey} instance corresponding to this instance.
     *
     * @return  A {@link VtnPortKey} instance.
     */
    public VtnPortKey getVtnPortKey() {
        return new VtnPortKey(getNodeConnectorId());
    }

    /**
     * Return an instance identifier that specifies VTN port instance.
     *
     * @return  An {@link InstanceIdentifier} instance.
     */
    public InstanceIdentifier<VtnPort> getVtnPortIdentifier() {
        return getVtnPortIdentifierBuilder().build();
    }

    /**
     * Return an instance identifier that specifies a link connected to the
     * switch port corresponding to this instance.
     *
     * @param linkId  A {@link LinkId} instance which represents an
     *                inter-switch link ID.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public InstanceIdentifier<PortLink> getPortLinkIdentifier(LinkId linkId) {
        return getVtnPortIdentifierBuilder().
            child(PortLink.class, new PortLinkKey(linkId)).build();
    }

    /**
     * Return an AD-SAL node connector.
     *
     * @return  An AD-SAL node connector that represents this instance.
     */
    public org.opendaylight.controller.sal.core.NodeConnector getAdNodeConnector() {
        if (adNodeConnector == null) {
            org.opendaylight.controller.sal.core.Node node = getAdNode();
            Short num = Short.valueOf((short)portNumber);
            adNodeConnector =
                NodeConnectorCreator.createOFNodeConnector(num, node);
        }

        return adNodeConnector;
    }

    /**
     * Return an instance identifier builder that contains an instance
     * identifier for a VTN port.
     *
     * @return  An instance identifier builder for a VTN port.
     *
     */
    private InstanceIdentifierBuilder<VtnPort> getVtnPortIdentifierBuilder() {
        return getVtnNodeIdentifierBuilder().
            child(VtnPort.class, getVtnPortKey());
    }

    /**
     * Return a string which identifies the switch corresponding to this
     * instance.
     *
     * @return  A string which identifies the switch.
     */
    @Override
    public String toNodeString() {
        return toNodeStringBuilder().toString();
    }

    /**
     * Return a {@link StringBuilder} instance that contains a string
     * representation of this instance.
     *
     * @return  A {@link StringBuilder} instance.
     */
    @Override
    protected StringBuilder toStringBuilder() {
        return toNodeStringBuilder().append(MD_ID_SEPARATOR).
            append(portNumber);
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (!super.equals(o)) {
            return false;
        }

        SalPort sport = (SalPort)o;
        return (portNumber == sport.portNumber);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() + NumberUtils.hashCode(portNumber);
    }
}
