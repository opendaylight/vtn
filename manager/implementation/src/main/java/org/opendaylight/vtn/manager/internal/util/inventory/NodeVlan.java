/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import java.util.Objects;

import org.opendaylight.vtn.manager.internal.util.VlanDescParser;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

/**
 * {@code NodeVlan} class represents a pair of the physical switch and a
 * VLAN ID.
 */
public final class NodeVlan {
    /**
     * A {@link SalNode} associated with the physical switch.
     */
    private final SalNode  node;

    /**
     * VLAN ID.
     */
    private final int  vlanId;

    /**
     * Cache for a string representation.
     */
    private String  stringCache;

    /**
     * Cache for a hash code.
     */
    private int  hash;

    /**
     * Construct a new instance which specifies the VLAN network.
     *
     * @param snode A {@link SalNode} associated with the physical switch.
     *              Specifying {@code null} means that no physical switch is
     *              specified.
     * @param vid   VLAN ID.
     */
    public NodeVlan(SalNode snode, int vid) {
        node = snode;
        vlanId = vid;
    }

    /**
     * Construct a new instance from the given string.
     *
     * @param value  A string concatenated a node-id and a VLAN ID with "@".
     * @throws RpcException
     *    An invalid value is specified to {@code value}.
     */
    public NodeVlan(String value) throws RpcException {
        VlanDescParser parser = new VlanDescParser(value, "node-vlan");
        String id = parser.getIdentifier();
        if (id == null) {
            node = null;
        } else {
            node = SalNode.create(id);
            if (node == null) {
                throw RpcException.getBadArgumentException(
                    "Invalid node-id in node-vlan: " + value);
            }
        }

        vlanId = parser.getVlanId();
        stringCache = value;
    }

    /**
     * Return the node associated with the physical switch.
     *
     * @return  A {@link SalNode} object.
     *          {@code null} is returned if no physical switch is specified.
     */
    public SalNode getNode() {
        return node;
    }

    /**
     * Return the VLAN ID.
     *
     * @return  VLAN ID.
     */
    public int getVlanId() {
        return vlanId;
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        boolean ret = (o == this);
        if (!ret && o != null && getClass().equals(o.getClass())) {
            NodeVlan nv = (NodeVlan)o;
            ret = (vlanId == nv.vlanId && Objects.equals(node, nv.node));
        }

        return ret;
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int  h = hash;
        if (h == 0) {
            h = Objects.hash(NodeVlan.class, node, vlanId);
            hash = h;
        }

        return h;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        String value = stringCache;
        if (value == null) {
            StringBuilder builder = new StringBuilder();
            if (node != null) {
                builder.append(node);
            }

            value = builder.append(VlanDescParser.SEPARATOR).
                append(vlanId).toString();
            stringCache = value;
        }

        return value;
    }
}
