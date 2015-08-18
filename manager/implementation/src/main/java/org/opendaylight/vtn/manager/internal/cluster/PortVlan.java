/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * {@code PortVlan} class represents a pair of the physical switch port and
 * the VLAN ID.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class PortVlan implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -7201707889447361884L;

    /**
     * Node connector associated with the physical switch port.
     */
    private final NodeConnector  nodeConnector;

    /**
     * VLAN ID.
     */
    private final short  vlan;

    /**
     * Construct a new instance.
     *
     * @param nc    Node connector associated with the physical switch port.
     *              Specifying {@code null} results in undefined behavior.
     * @param vlan  VLAN ID.
     */
    public PortVlan(NodeConnector nc, short vlan) {
        this.nodeConnector = nc;
        this.vlan = vlan;
    }

    /**
     * Return the node connector associated with the physical switch port.
     *
     * @return  A node connector.
     */
    public NodeConnector getNodeConnector() {
        return nodeConnector;
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
        if (!(o instanceof PortVlan)) {
            return false;
        }

        PortVlan pvlan = (PortVlan)o;
        return (vlan == pvlan.vlan &&
                nodeConnector.equals(pvlan.nodeConnector));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return vlan + nodeConnector.hashCode();
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("PortVlan[connector=");
        builder.append(nodeConnector.toString()).
            append(",vlan=").append((int)vlan).append(']');

        return builder.toString();
    }
}
