/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.flow.match;

import static org.junit.Assert.assertEquals;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnPortRange;
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
 * {@code PortRange} describes the range of IP transport layer protocol
 * such as TCP.
 */
public final class PortRange {
    /**
     * The minimum value (inclusive) in the range of TCP/UDP port numbers
     * to match against packets.
     */
    private Integer  portFrom;

    /**
     * The maximum value (inclusive) in the range of TCP/UDP port numbers
     * to match against packets.
     */
    private Integer  portTo;

    /**
     * Create a new port-number instance.
     *
     * @param port  The port number.
     * @return  A {@link PortNumber} instance if {@code port} is not
     *          {@code null}. {@code null} otherwise.
     */
    public static PortNumber toPortNumber(Integer port) {
        return (port == null) ? null : new PortNumber(port);
    }

    /**
     * Construct a new instance that specifies the given port number.
     *
     * @param port  The port number.
     */
    public PortRange(Integer port) {
        portFrom = port;
        portTo = null;
    }

    /**
     * Construct a new instance that specifies the range of port number.
     *
     * @param from  The minimum (inclusive) port number in the range of
     *              port numbers.
     * @param to    The maximum (inclusive) port number in the range of
     *              port numbers.
     */
    public PortRange(Integer from, Integer to) {
        portFrom = from;
        portTo = to;
    }

    /**
     * Return the range of TCP source port number.
     *
     * @return  A {@link TcpSourceRange} instance.
     */
    public TcpSourceRange getTcpSourceRange() {
        return new TcpSourceRangeBuilder().
            setPortFrom(getPortNumberFrom()).
            setPortTo(getPortNumberTo()).
            build();
    }

    /**
     * Return the range of TCP destination port number.
     *
     * @return  A {@link TcpDestinationRange} instance.
     */
    public TcpDestinationRange getTcpDestinationRange() {
        return new TcpDestinationRangeBuilder().
            setPortFrom(getPortNumberFrom()).
            setPortTo(getPortNumberTo()).
            build();
    }

    /**
     * Return the range of UDP source port number.
     *
     * @return  A {@link UdpSourceRange} instance.
     */
    public UdpSourceRange getUdpSourceRange() {
        return new UdpSourceRangeBuilder().
            setPortFrom(getPortNumberFrom()).
            setPortTo(getPortNumberTo()).
            build();
    }

    /**
     * Return the range of UDP destination port number.
     *
     * @return  A {@link UdpDestinationRange} instance.
     */
    public UdpDestinationRange getUdpDestinationRange() {
        return new UdpDestinationRangeBuilder().
            setPortFrom(getPortNumberFrom()).
            setPortTo(getPortNumberTo()).
            build();
    }

    /**
     * Return the minimum (inclusive) value in the range of port numbers
     * to match against packets.
     *
     * @return  The minimum value in the range of port numbers.
     */
    public Integer getPortFrom() {
        return portFrom;
    }

    /**
     * Set the minimum (inclusive) value in the range of port numbers
     * to match against packets.
     *
     * @param port  The minimum value in the range of port numbers.
     * @return  This instance.
     */
    public PortRange setPortFrom(Integer port) {
        portFrom = port;
        return this;
    }

    /**
     * Return the maximum (inclusive) value in the range of port numbers
     * to match against packets.
     *
     * @return  The maximum value in the range of port numbers.
     */
    public Integer getPortTo() {
        return portTo;
    }

    /**
     * Set the maximum (inclusive) value in the range of port numbers
     * to match against packets.
     *
     * @param port  The maximum value in the range of port numbers.
     * @return  This instance.
     */
    public PortRange setPortTo(Integer port) {
        portTo = port;
        return this;
    }

    /**
     * Return the minimum (inclusive) value in the range of port numbers
     * to match against packets.
     *
     * @return  A {@link PortNumber} instance that indicates the minimum value
     *          in the range of port numbers.
     */
    public PortNumber getPortNumberFrom() {
        return toPortNumber(portFrom);
    }

    /**
     * Return the maximum (inclusive) value in the range of port numbers
     * to match against packets.
     *
     * @return  A {@link PortNumber} instance that indicates the maximum value
     *          in the range of port numbers.
     */
    public PortNumber getPortNumberTo() {
        return toPortNumber(portTo);
    }

    /**
     * Verify the given vtn-port-range value.
     *
     * @param vrange  A {@link VtnPortRange} instance.
     */
    public void verify(VtnPortRange vrange) {
        assertEquals(portFrom, vrange.getPortFrom().getValue());

        Integer to = portTo;
        if (to == null) {
            to = portFrom;
        }
        assertEquals(to, vrange.getPortTo().getValue());
    }
}
