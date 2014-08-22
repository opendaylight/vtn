/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.PortMatch;

import org.opendaylight.vtn.manager.internal.MiscUtils;

import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * {@code L4PortMatch} describes the range of TCP/UDP port number.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public final class L4PortMatch implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 5154123070973523206L;

    /**
     * The minimum value (inclusive) in the range of TCP/UDP port numbers
     * to match against packets.
     */
    private final int  portFrom;

    /**
     * The maximum value (inclusive) in the range of TCP/UDP port numbers
     * to match against packets.
     */
    private final int  portTo;

    /**
     * Construct a new instance.
     *
     * @param match  A {@link PortMatch} instance.
     * @throws NullPointerException
     *    {@code match} is {@code null}.
     * @throws VTNException
     *    {@code match} contains invalid value.
     */
    public L4PortMatch(PortMatch match) throws VTNException {
        Integer from = match.getPortFrom();
        if (from == null) {
            throw new VTNException(StatusCode.BADREQUEST,
                                   "\"from\" is not specified.");
        }

        portFrom = from.intValue();
        checkPort(portFrom, "from");

        Integer to = match.getPortTo();
        if (to == null) {
            portTo = portFrom;
        } else {
            portTo = to.intValue();
            checkPort(portTo, "to");
            if (from > to) {
                StringBuilder builder =
                    new StringBuilder("Invalid port range: from=");
                builder.append(portFrom).append(", to=").append(portTo);
                throw new VTNException(StatusCode.BADREQUEST,
                                       builder.toString());
            }
        }
    }

    /**
     * Return the minimum (inclusive) value in the range of TCP/UDP port
     * numbers to match against packets.
     *
     * @return  The minimum value in the range of TCP/UDP port numbers.
     */
    public int getPortFrom() {
        return portFrom;
    }

    /**
     * Return the maximum (inclusive) value in the range of TCP/UDP port
     * numbers to match against packets.
     *
     * @return  The maximum value in the range of TCP/UDP port numbers.
     */
    public int getPortTo() {
        return portTo;
    }

    /**
     * Determine whether the given port number is in the range configured
     * in this instance.
     *
     * @param port  A port number to be tested.
     * @return  {@code true} is returned if the specified port number is
     *          in the range of port numbers specified by this instance.
     *          Otherwise {@code false} is returned.
     */
    public boolean match(int port) {
        return (port >= portFrom && port <= portTo);
    }

    /**
     * Validate the given port number.
     *
     * @param port  A port number.
     * @param desc  A brief description about the value.
     * @throws VTNException
     *    An invalid port number is specified.
     */
    private void checkPort(int port, String desc) throws VTNException {
        if (!MiscUtils.isPortNumberValid(port)) {
            StringBuilder builder = new StringBuilder(desc);
            builder.append(": Invalid port number: ").append(port);
            throw new VTNException(StatusCode.BADREQUEST, builder.toString());
        }
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
        if (!(o instanceof L4PortMatch)) {
            return false;
        }

        L4PortMatch match = (L4PortMatch)o;
        return (portFrom == match.portFrom && portTo == match.portTo);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return portFrom + portTo * 17;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        builder.append(portFrom);
        if (portTo != portFrom) {
            builder.append('-').append(portTo);
        }

        return builder.toString();
    }
}
