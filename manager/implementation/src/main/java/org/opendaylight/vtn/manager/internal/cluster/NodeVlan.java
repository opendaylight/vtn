/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;

import org.opendaylight.controller.sal.core.Node;

/**
 * {@code NodeVlan} class represents a pair of the physical switch and the
 * VLAN ID.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class NodeVlan implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -4658190509280565072L;

    /**
     * A {@link Node} associated with the physical switch.
     */
    private final Node  node;

    /**
     * VLAN ID.
     */
    private final short  vlan;

    /**
     * Construct a new instance which specifies the VLAN network.
     *
     * @param node  A {@link Node} associated with the physical switch.
     *              Specifying {@code null} means that no physical switch is
     *              specified.
     * @param vlan  VLAN ID.
     */
    public NodeVlan(Node node, short vlan) {
        this.node = node;
        this.vlan = vlan;
    }

    /**
     * Return the node associated with the physical switch.
     *
     * @return  A {@link Node} object.
     *          {@code null} is returned if no physical switch is specified.
     */
    public Node getNode() {
        return node;
    }

    /**
     * Return the VLAN ID.
     *
     * @return  VLAN ID.
     */
    public short getVlan() {
        return vlan;
    }

    /**
     * Append human readable strings which represents the contents of this
     * object to the specified {@link StringBuilder}.
     *
     * @param builder  A {@link StringBuilder} instance.
     */
    public void appendContents(StringBuilder builder) {
        if (node != null) {
            builder.append("node=").append(node.toString()).append(',');
        }
        builder.append("vlan=").append((int)vlan);
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
        if (!(o instanceof NodeVlan)) {
            return false;
        }

        NodeVlan nvlan = (NodeVlan)o;
        if (vlan != nvlan.vlan) {
            return false;
        }

        if (node == null) {
            return (nvlan.node == null);
        }

        return node.equals(nvlan.node);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int  h = (int)vlan;
        if (node != null) {
            h ^= node.hashCode();
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
        StringBuilder builder = new StringBuilder("NodeVlan[");
        appendContents(builder);
        builder.append(']');

        return builder.toString();
    }
}
