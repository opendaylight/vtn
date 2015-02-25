/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeConnectorRef;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.NodeId;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.Nodes;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnector;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.node.NodeConnectorKey;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.Node;
import org.opendaylight.yang.gen.v1.urn.opendaylight.inventory.rev130819.nodes.NodeKey;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * {@code OfMockUtils} contains static utility methods.
 */
public final class OfMockUtils {
    /**
     * A bitmask for a byte value.
     */
    public static final int  MASK_BYTE = 0xff;

    /**
     * A bitmask for a short value.
     */
    public static final int  MASK_SHORT = 0xffff;

    /**
     * Private constructor that protects this class from instantiating.
     */
    private OfMockUtils() {}

    /**
     * Create a node identifier from the given port identifier.
     *
     * @param pid  The port identifier.
     * @return  The node identifier string.
     */
    public static String getNodeIdentifier(String pid) {
        int idx = pid.lastIndexOf(':');
        if (idx < 0) {
            throw new IllegalArgumentException(
                "Invalid port identifier: " + pid);
        }

        return pid.substring(0, idx);
    }

    /**
     * Return the port identifier string for the specified port.
     *
     * @param nid  The node identifier.
     * @param num  The port number.
     * @return  A port identifier string.
     */
    public static String getPortIdentifier(String nid, long num) {
        StringBuilder builder = new StringBuilder(nid).
            append(OfMockService.ID_SEPARATOR).append(num);
        return builder.toString();
    }

    /**
     * Return the port identifier string in the given node connector ID.
     *
     * @param id  A {@link NodeConnectorId} instance.
     * @return  A port identifier string.
     *          {@code null} is returned if the port identifier string is
     *          not present.
     */
    public static String getPortIdentifier(NodeConnectorId id) {
        return (id == null) ? null : id.getValue();
    }

    /**
     * Return the MD-SAL node connector indentifier configured in the given
     * node connector reference.
     *
     * @param ref  A {@link NodeConnectorRef} instance.
     * @return  The MD-SAL node connector identifier.
     *          {@code null} is returned if not available.
     */
    public static String getPortIdentifier(NodeConnectorRef ref) {
        if (ref == null) {
            return null;
        }

        InstanceIdentifier<?> path = ref.getValue();
        if (path == null) {
            return null;
        }

        NodeConnectorKey key =
            path.firstKeyOf(NodeConnector.class, NodeConnectorKey.class);
        if (key == null) {
            return null;
        }

        return getPortIdentifier(key.getId());
    }

    /**
     * Create a instance identifier which specifies the given switch port.
     *
     * @param pid  The port identifier.
     * @return  An {@link InstanceIdentifier} instance.
     */
    public static InstanceIdentifier<NodeConnector> getPortPath(String pid) {
        String nid = OfMockUtils.getNodeIdentifier(pid);
        NodeConnectorId ncid = new NodeConnectorId(pid);

        return InstanceIdentifier.builder(Nodes.class).
            child(Node.class, new NodeKey(new NodeId(nid))).
            child(NodeConnector.class, new NodeConnectorKey(ncid)).build();
    }

    /**
     * Convert the given MD-SAL MAC address into a byte array.
     *
     * @param maddr  A {@link MacAddress} instance.
     * @return  A byte array which represents the given MAC address.
     *          {@code null} is returned if {@code null} is specified.
     */
    public static byte[] getMacAddress(MacAddress maddr) {
        return (maddr == null) ? null : new EtherAddress(maddr).getBytes();
    }
}
