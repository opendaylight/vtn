/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.util.Objects;

import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.flow.cond.PortMatch;
import org.opendaylight.vtn.manager.flow.cond.PortProtoMatch;

import org.opendaylight.controller.sal.utils.Status;
import org.opendaylight.controller.sal.utils.StatusCode;

/**
 * {@code PortProtoMatchImpl} describes the condition to match layer 4 protocol
 * header fields, which identifies the service using 16-bit port number.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public abstract class PortProtoMatchImpl extends L4MatchImpl {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -5599948746651922234L;

    /**
     * A {@link L4PortMatch} instance which describes the range of
     * source port numbers to match against packets.
     */
    private final L4PortMatch  sourcePort;

    /**
     * A {@link L4PortMatch} instance which describes the range of
     * destination port numbers to match against packets.
     */
    private final L4PortMatch  destinationPort;

    /**
     * Construct a new instance.
     *
     * @param match  An {@link PortProtoMatch} instance.
     * @throws NullPointerException
     *    {@code match} is {@code null}.
     * @throws VTNException
     *    {@code match} contains invalid value.
     */
    protected PortProtoMatchImpl(PortProtoMatch match) throws VTNException {
        sourcePort = getL4PortMatch(match.getSourcePort(), "source");
        destinationPort = getL4PortMatch(match.getDestinationPort(),
                                         "destination");
    }

    /**
     * Return a {@link L4PortMatch} instance which describes the range of
     * source port numbers to match against packets.
     *
     * @return  A {@link L4PortMatch} instances which describes the source
     *          port numbers to match.
     *          {@code null} is returned if no source port number is specified.
     */
    public final L4PortMatch getSourcePort() {
        return sourcePort;
    }

    /**
     * Return a {@link L4PortMatch} instance which describes the range of
     * destination port numbers to match against packets.
     *
     * @return  A {@link L4PortMatch} instance which describes the destination
     *          port numbers to match.
     *          {@code null} is returned if no destination port number is
     *          specified.
     */
    public final L4PortMatch getDestinationPort() {
        return destinationPort;
    }

    /**
     * Return a {@link PortMatch} instance which represents the specified
     * {@link L4PortMatch} instance.
     *
     * @param match  A {@link L4PortMatch} instance.
     * @return  A {@link PortMatch} instance.
     *          {@code null} is returned if {@code null} is specified.
     */
    protected PortMatch getPortMatch(L4PortMatch match) {
        if (match == null) {
            return null;
        }

        int from = match.getPortFrom();
        int to = match.getPortTo();
        return (from == to)
            ? new PortMatch(Integer.valueOf(from))
            : new PortMatch(Integer.valueOf(from), Integer.valueOf(to));
    }

    /**
     * Return a {@link L4PortMatch} instance which represents the contents
     * of the specified {@link PortMatch} instance.
     *
     * @param match  A {@link PortMatch} instance.
     * @param desc   A brief description about the value.
     * @return  A {@link L4PortMatch} instance.
     *          {@code null} is returned if {@code match} is {@code null}.
     * @throws VTNException
     *    {@code match} contains invalid value.
     */
    private L4PortMatch getL4PortMatch(PortMatch match, String desc)
        throws VTNException {
        if (match == null) {
            return null;
        }

        try {
            return new L4PortMatch(match);
        } catch (VTNException e) {
            StringBuilder builder = new StringBuilder(desc);
            builder.append(": ").append(e.getStatus().getDescription());
            Status st = new Status(StatusCode.BADREQUEST, builder.toString());
            throw new VTNException(st, e);
        }
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

        PortProtoMatchImpl match = (PortProtoMatchImpl)o;
        return (Objects.equals(sourcePort, match.sourcePort) &&
                Objects.equals(destinationPort, match.destinationPort));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        return Objects.hash(sourcePort, destinationPort);
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
