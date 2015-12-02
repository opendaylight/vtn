/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import org.opendaylight.vtn.manager.internal.XmlNode;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.tcp.match.fields.TcpDestinationRange;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.tcp.match.fields.TcpDestinationRangeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.tcp.match.fields.TcpSourceRange;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.tcp.match.fields.TcpSourceRangeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.udp.match.fields.UdpDestinationRange;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.udp.match.fields.UdpDestinationRangeBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.udp.match.fields.UdpSourceRange;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.vtn.udp.match.fields.UdpSourceRangeBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * {@code PortRangeParams} describes parameters for conditions which specifies
 * the range of transport port numbers.
 */
public final class PortRangeParams implements Cloneable {
    /**
     * The minimum value in the range of port numbers.
     */
    private Integer  portFrom;

    /**
     * The maximum value in the range of port numbers.
     */
    private Integer  portTo;

    /**
     * Construct a new instance.
     */
    public PortRangeParams() {
    }

    /**
     * Construct a new instance.
     *
     * @param from  The minimum value in the range of port numbers.
     */
    public PortRangeParams(Integer from) {
        portFrom = from;
    }

    /**
     * Construct a new instance.
     *
     * @param from  The minimum value in the range of port numbers.
     * @param to    The maximum value in the range of port numbers.
     */
    public PortRangeParams(Integer from, Integer to) {
        portFrom = from;
        portTo = to;
    }

    /**
     * Return a {@link PortNumber} instance which contains the given port
     * number.
     *
     * @param port  A port number.
     * @return  A {@link PortNumber} instance or {@code null}.
     */
    private static PortNumber getPortNumber(Integer port) {
        return (port == null) ? null : new PortNumber(port);
    }

    /**
     * Return the minimum value in the range of the port numbers.
     *
     * @return  The minimum value in the range of the port numbers.
     */
    public Integer getPortFrom() {
        return portFrom;
    }

    /**
     * Set the minimum value in the range of the port numbers.
     *
     * @param port  The minimum value in the range of the port numbers.
     * @return  This instance.
     */
    public PortRangeParams setPortFrom(Integer port) {
        portFrom = port;
        return this;
    }

    /**
     * Return the maximum value in the range of the port numbers.
     *
     * @return  The maximum value in the range of the port numbers.
     */
    public Integer getPortTo() {
        return portTo;
    }

    /**
     * Set the maximum value in the range of the port numbers.
     *
     * @param port  The maximum value in the range of the port numbers.
     * @return  This instance.
     */
    public PortRangeParams setPortTo(Integer port) {
        portTo = port;
        return this;
    }

    /**
     * Reset to the initial state.
     *
     * @return  This instance.
     */
    public PortRangeParams reset() {
        portFrom = null;
        portTo = null;
        return this;
    }

    /**
     * Construct a new {@link TcpSourceRange} instance.
     *
     * @return  A {@link TcpSourceRange} instance.
     */
    public TcpSourceRange toTcpSourceRange() {
        return toTcpSourceRange(false);
    }

    /**
     * Construct a new {@link TcpSourceRange} instance.
     *
     * @param comp  Complete the settings if {@code true}.
     * @return  A {@link TcpSourceRange} instance.
     */
    public TcpSourceRange toTcpSourceRange(boolean comp) {
        Integer to = portTo;
        if (to == null && comp) {
            to = portFrom;
        }
        return new TcpSourceRangeBuilder().
            setPortFrom(getPortNumber(portFrom)).
            setPortTo(getPortNumber(to)).build();
    }

    /**
     * Construct a new {@link TcpDestinationRange} instance.
     *
     * @return  A {@link TcpDestinationRange} instance.
     */
    public TcpDestinationRange toTcpDestinationRange() {
        return toTcpDestinationRange(false);
    }

    /**
     * Construct a new {@link TcpDestinationRange} instance.
     *
     * @param comp  Complete the settings if {@code true}.
     * @return  A {@link TcpDestinationRange} instance.
     */
    public TcpDestinationRange toTcpDestinationRange(boolean comp) {
        Integer to = portTo;
        if (to == null && comp) {
            to = portFrom;
        }
        return new TcpDestinationRangeBuilder().
            setPortFrom(getPortNumber(portFrom)).
            setPortTo(getPortNumber(to)).build();
    }

    /**
     * Construct a new {@link UdpSourceRange} instance.
     *
     * @return  A {@link UdpSourceRange} instance.
     */
    public UdpSourceRange toUdpSourceRange() {
        return toUdpSourceRange(false);
    }

    /**
     * Construct a new {@link UdpSourceRange} instance.
     *
     * @param comp  Complete the settings if {@code true}.
     * @return  A {@link UdpSourceRange} instance.
     */
    public UdpSourceRange toUdpSourceRange(boolean comp) {
        Integer to = portTo;
        if (to == null && comp) {
            to = portFrom;
        }
        return new UdpSourceRangeBuilder().
            setPortFrom(getPortNumber(portFrom)).
            setPortTo(getPortNumber(to)).build();
    }

    /**
     * Construct a new {@link UdpDestinationRange} instance.
     *
     * @return  A {@link UdpDestinationRange} instance.
     */
    public UdpDestinationRange toUdpDestinationRange() {
        return new UdpDestinationRangeBuilder().
            setPortFrom(getPortNumber(portFrom)).
            setPortTo(getPortNumber(portTo)).build();
    }

    /**
     * Construct a new {@link UdpDestinationRange} instance.
     *
     * @param comp  Complete the settings if {@code true}.
     * @return  A {@link UdpDestinationRange} instance.
     */
    public UdpDestinationRange toUdpDestinationRange(boolean comp) {
        Integer to = portTo;
        if (to == null && comp) {
            to = portFrom;
        }
        return new UdpDestinationRangeBuilder().
            setPortFrom(getPortNumber(portFrom)).
            setPortTo(getPortNumber(to)).build();
    }

    /**
     * Return a {@link XmlNode} instance to be mapped to a {@link VTNPortRange}
     * instance.
     *
     * @param name  The name of the root node.
     * @return  A {@link XmlNode} instance.
     */
    public XmlNode toXmlNode(String name) {
        XmlNode root = new XmlNode(name);
        if (portFrom != null) {
            root.add(new XmlNode("port-from", portFrom));
        }
        if (portTo != null) {
            root.add(new XmlNode("port-to", portTo));
        }

        return root;
    }

    // Cloneable

    /**
     * Return a shallow copy of this instance.
     *
     * @return  A shallow copy of this instance.
     */
    @Override
    public PortRangeParams clone() {
        try {
            return (PortRangeParams)super.clone();
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed.", e);
        }
    }
}
