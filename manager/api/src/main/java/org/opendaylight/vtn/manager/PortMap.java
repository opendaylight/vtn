/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;
import java.util.Objects;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * {@code PortMap} class describes information about the
 * {@linkplain <a href="package-summary.html#port-map">port mapping</a>}, which
 * maps a physical switch port to a
 * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
 * in a virtual node.
 *
 * <p>
 *   The VTN Manager passes the port mapping information to other components
 *   by passing {@code PortMap} object.
 * </p>
 *
 * @see  <a href="package-summary.html#port-map">Port mapping</a>
 */
public class PortMap implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 9006400881038730332L;

    /**
     * {@linkplain <a href="package-summary.html#port-map">Port mapping</a>}
     * configuration.
     */
    private final PortMapConfig  config;

    /**
     * Node connector corresponding to the the physical switch port actually
     * mapped to the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     */
    private final NodeConnector  nodeConnector;

    /**
     * Construct a new object which represents information about the
     * {@linkplain <a href="package-summary.html#port-map">port mapping</a>}.
     *
     * @param pmconf  A {@link PortMapConfig} object which contains
     *                configuration information about port mapping.
     * @param nc
     *   A {@link NodeConnector} object corresponding to a physical switch port
     *   actually mapped to the
     *   {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     *   Specify {@code null} if there is no physical switch port that
     *   fulfills the conditions specified by {@code pmconf}.
     */
    public PortMap(PortMapConfig pmconf, NodeConnector nc) {
        this.config = pmconf;
        this.nodeConnector = nc;
    }

    /**
     * Return the configuration for
     * {@linkplain <a href="package-summary.html#port-map">port mapping</a>}.
     *
     * @return  A {@link PortMapConfig} object which contains port mapping
     *          configuration.
     */
    public PortMapConfig getConfig() {
        return config;
    }

    /**
     * Return the node connector corresponding to the physical switch port
     * actually mapped to the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     *
     * @return  A {@link NodeConnector} object corresponding to the physical
     *          swtich port actually mapped to the virtual interface.
     *          {@code null} is returned if no physical switch port meets the
     *          conditions specified by the port mapping configuration.
     */
    public NodeConnector getNodeConnector() {
        return nodeConnector;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code PortMap} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         {@link PortMapConfig} object which represents the
     *         {@linkplain <a href="package-summary.html#port-map">port mapping</a>}
     *         configuration.
     *       </li>
     *       <li>
     *         {@link NodeConnector} object corresponding to the physical
     *         switch port actually mapped to the
     *         {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     *       </li>
     *     </ul>
     *   </li>
     * </ul>
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
        return (Objects.equals(config, pmap.config) &&
                Objects.equals(nodeConnector, pmap.nodeConnector));
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
