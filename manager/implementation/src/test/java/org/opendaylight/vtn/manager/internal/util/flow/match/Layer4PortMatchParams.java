/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import org.opendaylight.vtn.manager.flow.cond.PortMatch;

import org.opendaylight.vtn.manager.internal.util.packet.Layer4PortHeader;

import org.opendaylight.vtn.manager.internal.XmlNode;

/**
 * {@code Layer4PortMatchParams} describes parameters for condition to match
 * against layer 4 protocol which identifies the service using 16-bit port
 * number.
 *
 * @param <T>  The type of this instance.
 */
public abstract class Layer4PortMatchParams<T extends Layer4PortMatchParams>
    extends Layer4MatchParams<T> implements Layer4PortHeader {
    /**
     * The range of source port numbers.
     */
    private PortRangeParams  sourcePort;

    /**
     * The range of destination port numbers.
     */
    private PortRangeParams  destinationPort;

    /**
     * Return the minimum value in the range of source port numbers.
     *
     * @return  The minimum value in the range of source port numbers.
     */
    public final Integer getSourcePortFrom() {
        return getSourcePortRange().getPortFrom();
    }

    /**
     * Set the minimum value in the range of source port number.
     *
     * @param port  The minimum value in the range of source port numbers.
     * @return  This instance.
     */
    public final T setSourcePortFrom(Integer port) {
        getSourcePortRange().setPortFrom(port);
        return getMatchType().cast(this);
    }

    /**
     * Return the maximum value in the range of source port numbers.
     *
     * @return  The maximum value in the range of source port numbers.
     */
    public final Integer getSourcePortTo() {
        return getSourcePortRange().getPortTo();
    }

    /**
     * Set the maximum value in the range of source port number.
     *
     * @param port  The maximum value in the range of source port numbers.
     * @return  This instance.
     */
    public final T setSourcePortTo(Integer port) {
        getSourcePortRange().setPortTo(port);
        return getMatchType().cast(this);
    }

    /**
     * Return the range of source port numbers.
     *
     * @return  A {@link PortRangeParams} instance.
     */
    public final PortRangeParams getSourcePortParams() {
        return sourcePort;
    }

    /**
     * Set the range of source port numbers.
     *
     * @param range  A {@link PortRangeParams} instance.
     * @return  This instance.
     */
    public final T setSourcePortParams(PortRangeParams range) {
        sourcePort = (range == null) ? null : range.clone();
        return getMatchType().cast(this);
    }

    /**
     * Return the minimum value in the range of destination port numbers.
     *
     * @return  The minimum value in the range of destination port numbers.
     */
    public final Integer getDestinationPortFrom() {
        return getDestinationPortRange().getPortFrom();
    }

    /**
     * Set the minimum value in the range of destination port number.
     *
     * @param port  The minimum value in the range of destination port numbers.
     * @return  This instance.
     */
    public final T setDestinationPortFrom(Integer port) {
        getDestinationPortRange().setPortFrom(port);
        return getMatchType().cast(this);
    }

    /**
     * Return the maximum value in the range of destination port numbers.
     *
     * @return  The maximum value in the range of destination port numbers.
     */
    public final Integer getDestinationPortTo() {
        return getDestinationPortRange().getPortTo();
    }

    /**
     * Set the maximum value in the range of destination port number.
     *
     * @param port  The maximum value in the range of destination port numbers.
     * @return  This instance.
     */
    public final T setDestinationPortTo(Integer port) {
        getDestinationPortRange().setPortTo(port);
        return getMatchType().cast(this);
    }

    /**
     * Return the range of destination port numbers.
     *
     * @return  A {@link PortRangeParams} instance.
     */
    public final PortRangeParams getDestinationPortParams() {
        return destinationPort;
    }

    /**
     * Set the range of destination port numbers.
     *
     * @param range  A {@link PortRangeParams} instance.
     * @return  This instance.
     */
    public final T setDestinationPortParams(PortRangeParams range) {
        destinationPort = (range == null) ? null : range.clone();
        return getMatchType().cast(this);
    }

    /**
     * Reset to the initial state.
     *
     * @return  This instance.
     */
    public final T reset() {
        sourcePort = null;
        destinationPort = null;
        return getMatchType().cast(this);
    }

    /**
     * Prepare the range of source port numbers.
     *
     * @return  A {@link PortRangeParams} instance.
     */
    protected final PortRangeParams getSourcePortRange() {
        PortRangeParams range = sourcePort;
        if (range == null) {
            range = new PortRangeParams();
            sourcePort = range;
        }
        return range;
    }

    /**
     * Prepare the range of destination port numbers.
     *
     * @return  A {@link PortRangeParams} instance.
     */
    protected final PortRangeParams getDestinationPortRange() {
        PortRangeParams range = destinationPort;
        if (range == null) {
            range = new PortRangeParams();
            destinationPort = range;
        }
        return range;
    }

    /**
     * Return a {@link PortMatch} instance which represents the range of
     * source port numbers.
     *
     * @return  A {@link PortMatch} instance or {@code null}.
     */
    public final PortMatch getSourcePortMatch() {
        PortRangeParams range = sourcePort;
        return (range == null) ? null : range.toPortMatch();
    }

    /**
     * Return a {@link PortMatch} instance which represents the range of
     * destination port numbers.
     *
     * @return  A {@link PortMatch} instance or {@code null}.
     */
    public final PortMatch getDestinationPortMatch() {
        PortRangeParams range = destinationPort;
        return (range == null) ? null : range.toPortMatch();
    }

    // Layer4MatchParams

    /**
     * {@inheritDoc}
     */
    @Override
    public final XmlNode toXmlNode(String name) {
        XmlNode root = new XmlNode(name);
        if (sourcePort != null) {
            root.add(sourcePort.toXmlNode("source-port"));
        }
        if (destinationPort != null) {
            root.add(destinationPort.toXmlNode("destination-port"));
        }

        return root;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final boolean isEmpty() {
        return (sourcePort == null && destinationPort == null);
    }

    // Layer4PortHeader

    /**
     * {@inheritDoc}
     */
    @Override
    public final int getSourcePort() {
        return getSourcePortRange().getPortFrom().intValue();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final void setSourcePort(int port) {
        getSourcePortRange().setPortFrom(port);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final int getDestinationPort() {
        return getDestinationPortRange().getPortFrom().intValue();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public final void setDestinationPort(int port) {
        getDestinationPortRange().setPortFrom(port);
    }

    // Cloneable

    /**
     * Return a deep copy of this instance.
     *
     * @return  A deep copy of this instance.
     */
    @Override
    public Layer4PortMatchParams clone() {
        Layer4PortMatchParams params = (Layer4PortMatchParams)super.clone();
        PortRangeParams range = sourcePort;
        if (range != null) {
            params.sourcePort = range.clone();
        }

        range = destinationPort;
        if (range != null) {
            params.destinationPort = range.clone();
        }

        return params;
    }
}
