/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow.match;

import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.flow.cond.PortMatch;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.ProtocolUtils;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.cond.rev150313.VtnPortRange;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.PortNumber;

/**
 * {@code VTNPortRange} describes the range of IP transport layer protocol
 * such as TCP.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
@XmlRootElement(name = "vtn-port-range")
@XmlAccessorType(XmlAccessType.NONE)
public final class VTNPortRange {
    /**
     * The minimum value (inclusive) in the range of TCP/UDP port numbers
     * to match against packets.
     */
    @XmlElement(name = "port-from")
    private Integer  portFrom;

    /**
     * The maximum value (inclusive) in the range of TCP/UDP port numbers
     * to match against packets.
     */
    @XmlElement(name = "port-to")
    private Integer  portTo;

    /**
     * Construct a new instance from the given {@link PortMatch} instance.
     *
     * @param pmatch  A {@link PortMatch} instance.
     * @return  A {@link VTNPortRange} instance if {@code pmatch} is not
     *          {@code null}. Otherwise {@code null}.
     * @throws RpcException
     *    {@code pmatch} contains invalid value.
     */
    public static VTNPortRange create(PortMatch pmatch) throws RpcException {
        return (pmatch == null) ? null : new VTNPortRange(pmatch);
    }

    /**
     * Construct a new instance from the given {@link VtnPortRange} instance.
     *
     * @param range  A {@link VtnPortRange} instance.
     * @return  A {@link VTNPortRange} instance if {@code pmatch} is not
     *          {@code null}. Otherwise {@code null}.
     * @throws RpcException
     *    {@code range} contains invalid value.
     */
    public static VTNPortRange create(VtnPortRange range) throws RpcException {
        return (range == null) ? null : new VTNPortRange(range);
    }

    /**
     * Construct a new instance which specifies the given port number
     * explicitly.
     *
     * @param port A {@link PortNumber} instance.
     * @return  A {@link VTNPortRange} instance if {@code port} is not
     *          {@code null}. Otherwise {@code null}.
     * @throws RpcException
     *    {@code port} contains invalid value.
     */
    public static VTNPortRange create(PortNumber port) throws RpcException {
        return (port == null) ? null : new VTNPortRange(port);
    }

    /**
     * Return a {@link PortMatch} instance which represents the given
     * condition.
     *
     * @param range  A {@link VTNPortRange} instance.
     * @return  A {@link PortMatch} instance if {@code range} is not
     *          {@code null}. Otherwise {@code null}.
     */
    public static PortMatch toPortMatch(VTNPortRange range) {
        return (range == null) ? null : range.toPortMatch();
    }

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private VTNPortRange() {
    }

    /**
     * Construct a new instance which specifies the given port number.
     *
     * @param port  The port number.
     * @throws RpcException  An invalid port number is specified.
     */
    public VTNPortRange(int port) throws RpcException {
        portFrom = Integer.valueOf(port);
        portTo = portFrom;
        checkPortFrom();
    }

    /**
     * Construct a new instance from the given {@link PortMatch} instance.
     *
     * @param pmatch  A {@link PortMatch} instance.
     * @throws NullPointerException
     *    {@code pmatch} is {@code null}.
     * @throws RpcException
     *    {@code pmatch} contains invalid value.
     */
    public VTNPortRange(PortMatch pmatch) throws RpcException {
        portFrom = pmatch.getPortFrom();
        checkPortFrom();
        ProtocolUtils.checkPortNumber(portFrom.intValue());

        Integer to = pmatch.getPortTo();
        if (to == null) {
            portTo = portFrom;
        } else {
            portTo = to;
            ProtocolUtils.checkPortNumber(portTo.intValue());
            checkPortRange();
        }
    }

    /**
     * Construct a new instance from the given {@link VtnPortRange} instance.
     *
     * @param range  A {@link VtnPortRange} instance.
     * @throws NullPointerException
     *    {@code range} is {@code null}.
     * @throws RpcException
     *    {@code range} contains invalid value.
     */
    public VTNPortRange(VtnPortRange range) throws RpcException {
        portFrom = ProtocolUtils.getPortNumber(range.getPortFrom());
        checkPortFrom();

        Integer to = ProtocolUtils.getPortNumber(range.getPortTo());
        if (to == null) {
            portTo = portFrom;
        } else {
            portTo = to;
            checkPortRange();
        }
    }

    /**
     * Construct a new instance which specifies the given port number
     * explicitly.
     *
     * @param port A {@link PortNumber} instance.
     * @throws RpcException
     *    {@code port} contains invalid value.
     */
    public VTNPortRange(PortNumber port) throws RpcException {
        portFrom = ProtocolUtils.getPortNumber(port);
        checkPortFrom();
        portTo = portFrom;
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
     * Return the maximum (inclusive) value in the range of port numbers
     * to match against packets.
     *
     * @return  The maximum value in the range of port numbers.
     */
    public Integer getPortTo() {
        return portTo;
    }

    /**
     * Return the minimum (inclusive) value in the range of port numbers
     * to match against packets.
     *
     * @return  A {@link PortNumber} instance that contains the minimum value
     *          in the range of port numbers.
     */
    public PortNumber getPortNumberFrom() {
        return new PortNumber(portFrom);
    }

    /**
     * Return the maximum (inclusive) value in the range of port numbers
     * to match against packets.
     *
     * @return  A {@link PortNumber} instance that contains the maximum value
     *          in the range of port numbers.
     */
    public PortNumber getPortNumberTo() {
        return new PortNumber(portTo);
    }

    /**
     * Return a {@link PortMatch} instance which represents this condition.
     *
     * @return  A {@link PortMatch} instance.
     */
    public PortMatch toPortMatch() {
        return new PortMatch(portFrom, portTo);
    }

    /**
     * Determine whether the given port number is contained by the port range
     * specified by this instance.
     *
     * @param port  A port number.
     * @return  {@code true} only if the given port number is contained by
     *          the port range.
     */
    public boolean match(int port) {
        int from = portFrom.intValue();
        int to = portTo.intValue();
        return (port >= from && port <= to);
    }

    /**
     * Store strings used to construct flow condition key.
     *
     * @param builder  A {@link StringBuilder} instance that contains strings
     *                 used to construct flow condition key.
     */
    public void setConditionKey(StringBuilder builder) {
        builder.append(portFrom);
        if (portFrom.intValue() != portTo.intValue()) {
            builder.append('-').append(portTo);
        }
    }

    /**
     * Verify the contents of this instance.
     *
     * @throws RpcException  Verifycation failed.
     */
    public void verify() throws RpcException {
        checkPortFrom();
        ProtocolUtils.checkPortNumber(portFrom.intValue());
        if (portTo == null) {
            portTo = portFrom;
        } else {
            ProtocolUtils.checkPortNumber(portTo.intValue());
            checkPortRange();
        }
    }

    /**
     * Ensure that the minimum value in the port range is configured.
     *
     * @throws RpcException
     *    The minimum value in the port range is missing.
     */
    private void checkPortFrom() throws RpcException {
        if (portFrom == null) {
            throw MiscUtils.getNullArgumentException("port-from");
        }
    }

    /**
     * Ensure the specified port range is valid.
     *
     * @throws RpcException
     *    The port range configured in this instance is invalid.
     */
    private void checkPortRange() throws RpcException {
        if (portFrom > portTo) {
            StringBuilder builder =
                new StringBuilder("Invalid port range: from=");
            builder.append(portFrom).append(", to=").append(portTo);
            throw RpcException.getBadArgumentException(builder.toString());
        }
    }

    // Objects

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

        VTNPortRange range = (VTNPortRange)o;
        return (Objects.equals(portFrom, range.portFrom) &&
                Objects.equals(portTo, range.portTo));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(portFrom, portTo);
    }
}
