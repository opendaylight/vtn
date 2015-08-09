/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.cond;

import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * This is a base class for classes which describe layer 4 protocol header
 * fields, which identifies the service using 16-bits port number.
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "portprotomatch")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({TcpMatch.class, UdpMatch.class})
public abstract class PortProtoMatch extends L4Match {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7505267902995843224L;

    /**
     * A {@link PortMatch} instance which describes the range of source port
     * numbers to match against packets.
     *
     * <ul>
     *   <li>If omitted, every source port number is matched.</li>
     * </ul>
     */
    @XmlElement(name = "src")
    private PortMatch  sourcePort;

    /**
     * A {@link PortMatch} instance which describes the range of destination
     * port numbers to match against packets.
     *
     * <ul>
     *   <li>If omitted, every destination port number is matched.</li>
     * </ul>
     */
    @XmlElement(name = "dst")
    private PortMatch  destinationPort;

    /**
     * Construct a new instance which describes every protocol header.
     */
    PortProtoMatch() {
    }

    /**
     * Construct a new instance with specifying source and destination
     * port number.
     *
     * @param src  A {@link PortMatch} instance which describes the range of
     *             source port numbers to match against packets.
     *             {@code null} means that every source port number is matched.
     * @param dst  A {@link PortMatch} instance which describes the range of
     *             destination port numbers to match against packets.
     *             {@code null} means that every destination port number is
     *             matched.
     */
    PortProtoMatch(PortMatch src, PortMatch dst) {
        sourcePort = src;
        destinationPort = dst;
    }

    /**
     * Construct a new instance with specifying source and destination
     * port number.
     *
     * @param src  A {@link Integer} instance which describes the source port
     *             source port number to match against packets.
     *             {@code null} means that every source port number is matched.
     * @param dst  A {@link Integer} instance which describes the destination
     *             port number to match against packets.
     *             {@code null} means that every destination port number is
     *             matched.
     */
    PortProtoMatch(Integer src, Integer dst) {
        if (src != null) {
            sourcePort = new PortMatch(src);
        }
        if (dst != null) {
            destinationPort = new PortMatch(dst);
        }
    }

    /**
     * Construct a new instance with specifying source and destination
     * port number.
     *
     * @param src  A {@link Short} instance which describes the source port
     *             source port number to match against packets.
     *             {@code null} means that every source port number is matched.
     * @param dst  A {@link Short} instance which describes the destination
     *             port number to match against packets.
     *             {@code null} means that every destination port number is
     *             matched.
     */
    PortProtoMatch(Short src, Short dst) {
        if (src != null) {
            sourcePort = new PortMatch(src);
        }
        if (dst != null) {
            destinationPort = new PortMatch(dst);
        }
    }

    /**
     * Return a {@link PortMatch} instance which describes the range of
     * source port numbers to match against packets.
     *
     * @return  A {@link PortMatch} instances which describes the source
     *          port numbers to match.
     *          {@code null} is returned if no source port number is specified.
     */
    public final PortMatch getSourcePort() {
        return sourcePort;
    }

    /**
     * Return a {@link PortMatch} instance which describes the range of
     * destination port numbers to match against packets.
     *
     * @return  A {@link PortMatch} instance which describes the destination
     *          port numbers to match.
     *          {@code null} is returned if no destination port number is
     *          specified.
     */
    public final PortMatch getDestinationPort() {
        return destinationPort;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public final boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        PortProtoMatch ppm = (PortProtoMatch)o;
        return (Objects.equals(sourcePort, ppm.sourcePort) &&
                Objects.equals(destinationPort, ppm.destinationPort));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        int h = getClass().getName().hashCode();
        return h + Objects.hash(sourcePort, destinationPort);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public final String toString() {
        StringBuilder builder = new StringBuilder(getClass().getSimpleName());
        builder.append('[');
        String sep = "";
        if (sourcePort != null) {
            builder.append("src=").append(sourcePort.toString());
            sep = ",";
        }
        if (destinationPort != null) {
            builder.append(sep).append("dst=").
                append(destinationPort.toString());
        }
        builder.append(']');

        return builder.toString();
    }
}
