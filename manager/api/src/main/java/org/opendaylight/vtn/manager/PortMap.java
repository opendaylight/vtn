/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * {@code PortMap} class describes information about mapping between virtual
 * bridge interface and physical switch port.
 */
public class PortMap implements Serializable {
    private final static long serialVersionUID = -7908977793022302055L;

    /**
     * Port mapping configuration.
     */
    private final PortMapConfig  config;

    /**
     * Node connector associated with the switch port actually mapped to the
     * virtual bridge interface.
     */
    private final NodeConnector  nodeConnector;

    /**
     * Construct a new port mapping information.
     *
     * @param pmconf  Port mapping configuration.
     * @param nc      Node connector assiciated with the switch port actually
     *                mapped to the virtual bridge interface.
     */
    public PortMap(PortMapConfig pmconf, NodeConnector nc) {
        this.config = pmconf;
        this.nodeConnector = nc;
    }

    /**
     * Return the configuration for port mapping.
     *
     * @return  Port mapping configuration.
     */
    public PortMapConfig getConfig() {
        return config;
    }

    /**
     * Return the node connector associated with the switch port actually
     * mapped to the virtual bridge interface.
     *
     * @return  A node connector. {@code null} is returned if the physical
     *          switch port which matches the configuration is not found.
     */
    public NodeConnector getNodeConnector() {
        return nodeConnector;
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
        if (!(o instanceof PortMap)) {
            return false;
        }

        PortMap pmap = (PortMap)o;
        if (config == null) {
            if (pmap.config != null) {
                return false;
            }
        } else if (!config.equals(pmap.config)) {
            return false;
        }

        if (nodeConnector == null) {
            return (pmap.nodeConnector == null);
        }

        return nodeConnector.equals(pmap.nodeConnector);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (config != null) {
            h ^= config.hashCode();
        }
        if (nodeConnector != null) {
            h ^= nodeConnector.hashCode();
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
        StringBuilder builder = new StringBuilder("PortMap[");
        if (config != null) {
            builder.append("config=").append(config.toString());
        }

        if (nodeConnector != null) {
            if (config != null) {
                builder.append(',');
            }
            builder.append("connector=").append(nodeConnector.toString());
        }

        builder.append(']');

        return builder.toString();
    }
}
