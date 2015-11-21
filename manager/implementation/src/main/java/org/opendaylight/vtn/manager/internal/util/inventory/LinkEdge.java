/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnSwitchLink;

import org.opendaylight.yang.gen.v1.urn.tbd.params.xml.ns.yang.network.topology.rev131021.network.topology.topology.Link;

/**
 * {@code LinkEdge} describes an edge of network topology.
 */
public final class LinkEdge {
    /**
     * The source switch port of this edge.
     */
    private final SalPort  sourcePort;

    /**
     * The destination switch port of this edge.
     */
    private final SalPort  destinationPort;

    /**
     * Construct a new instance.
     *
     * @param vlink  A {@link VtnSwitchLink} instance.
     * @throws NullPointerException
     *    {@code vlink} is {@code null}.
     * @throws IllegalArgumentException
     *    An invalid instance is specified to {@code vlink}.
     */
    public LinkEdge(VtnSwitchLink vlink) {
        sourcePort = SalPort.create(vlink.getSource());
        if (sourcePort == null) {
            throw new IllegalArgumentException(
                "Source port is not configured: " + vlink);
        }

        destinationPort = SalPort.create(vlink.getDestination());
        if (destinationPort == null) {
            throw new IllegalArgumentException(
                "Destination port is not configured: " + vlink);
        }
    }

    /**
     * Construct a new instance.
     *
     * @param link  A {@link Link} instance.
     * @throws NullPointerException
     *    {@code link} is {@code null}.
     * @throws IllegalArgumentException
     *    An invalid instance is specified to {@code link}.
     */
    public LinkEdge(Link link) {
        sourcePort = SalPort.create(link.getSource());
        if (sourcePort == null) {
            throw new IllegalArgumentException(
                "Source port is not configured: " + link);
        }

        destinationPort = SalPort.create(link.getDestination());
        if (destinationPort == null) {
            throw new IllegalArgumentException(
                "Destination port is not configured: " + link);
        }
    }

    /**
     * Return the source port of this edge.
     *
     * @return  A {@link SalPort} instance which represents the source
     *          switch port of the edge.
     */
    public SalPort getSourcePort() {
        return sourcePort;
    }

    /**
     * Return the destination port of this edge.
     *
     * @return  A {@link SalPort} instance which represents the destination
     *          switch port of the edge.
     */
    public SalPort getDestinationPort() {
        return destinationPort;
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
        if (o == this) {
            return true;
        }
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        LinkEdge le = (LinkEdge)o;
        return (sourcePort.equals(le.sourcePort) &&
                destinationPort.equals(le.destinationPort));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return sourcePort.hashCode() + destinationPort.hashCode() * HASH_PRIME;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("LinkEdge[");
        builder.append(sourcePort).append(" -> ").append(destinationPort).
            append(']');

        return builder.toString();
    }
}
