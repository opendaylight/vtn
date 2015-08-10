/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.ofmock;

import java.util.Objects;

/**
 * {@code OfMockLink} describes an inter-switch link between OpenFlow switches.
 */
public final class OfMockLink {
    /**
     * The source port identifier of the link.
     */
    private final String  sourcePort;

    /**
     * The destination port identifier of the link.
     */
    private final String  destinationPort;

    /**
     * Construct a new instance.
     *
     * @param src  The source port identifier of the link.
     * @param dst  The destination port identifier of the link.
     */
    public OfMockLink(String src, String dst) {
        sourcePort = src;
        destinationPort = dst;
    }

    /**
     * Return the source port identifier of the link.
     *
     * @return  The source port identifier of the link.
     */
    public String getSourcePort() {
        return sourcePort;
    }

    /**
     * Return the destination port identifier of the link.
     *
     * @return  The destination port identifier of the link.
     */
    public String getDestinationPort() {
        return destinationPort;
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
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        OfMockLink oml = (OfMockLink)o;
        return (sourcePort.equals(oml.sourcePort) &&
                destinationPort.equals(oml.destinationPort));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(sourcePort, destinationPort);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("OfMockLink[");
        builder.append(sourcePort).append(" -> ").append(destinationPort).
            append(']');

        return builder.toString();
    }
}
