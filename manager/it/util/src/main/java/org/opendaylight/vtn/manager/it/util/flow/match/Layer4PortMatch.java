/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.match;

/**
 * {@code Layer4PortMatch} describes the condition for IP port numbers
 * to match against packets.
 */
public abstract class Layer4PortMatch extends VTNLayer4Match {
    /**
     * The range of the source port number to match against packets.
     */
    private PortRange  sourcePort;

    /**
     * The range of the destination port number to match against packets.
     */
    private PortRange  destinationPort;

    /**
     * Construct an empty instance.
     */
    Layer4PortMatch() {
        sourcePort = null;
        destinationPort = null;
    }

    /**
     * Construct a new instance.
     *
     * @param src  The source port number to match against packet.
     *             {@code null} matches every source port number.
     * @param dst  The destination port number to match against packet.
     *             {@code null} matches every destination port number.
     */
    Layer4PortMatch(Integer src, Integer dst) {
        sourcePort = (src == null) ? null : new PortRange(src);
        destinationPort = (dst == null) ? null : new PortRange(dst);
    }

    /**
     * Construct a new instance.
     *
     * @param src  A {@link PortRange} instance which specifies the range
     *             of the source port number. {@code null} matches every
     *             source port number.
     * @param dst  A {@link PortRange} instance which specifies the range
     *             of the destination port number. {@code null} matches every
     *             destination port number.
     */
    Layer4PortMatch(PortRange src, PortRange dst) {
        sourcePort = src;
        destinationPort = dst;
    }

    /**
     * Return the range of the source port number to match against packet.
     *
     * @return  A {@link PortRange} instance if the range of the source
     *          port number is specified. {@code null} if not specified.
     */
    public final PortRange getSourcePort() {
        return sourcePort;
    }

    /**
     * Return the range of the destination port number to match against packet.
     *
     * @return  A {@link PortRange} instance if the range of the destination
     *          port number is specified. {@code null} if not specified.
     */
    public final PortRange getDestinationPort() {
        return destinationPort;
    }

    /**
     * Set the range of the source port number to match against packet.
     *
     * @param range  The range of the source port number.
     *               {@code null} matches every source port number.
     */
    protected final void setSource(PortRange range) {
        sourcePort = range;
    }

    /**
     * Set the range of the destination port number to match against packet.
     *
     * @param range  The range of the destination port number.
     *               {@code null} matches every destination port number.
     */
    protected final void setDestination(PortRange range) {
        destinationPort = range;
    }

    // VTNLayer4Match

    /**
     * {@inheritDoc}
     */
    @Override
    public final boolean isEmpty() {
        return (sourcePort == null && destinationPort == null);
    }
}
