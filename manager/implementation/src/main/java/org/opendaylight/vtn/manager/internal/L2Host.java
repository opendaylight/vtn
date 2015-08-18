/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.internal.cluster.MacVlan;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.controller.sal.core.NodeConnector;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * An instance of {@code L2Host} class specifies the location of layer 2 host.
 */
public final class L2Host {
    /**
     * A {@link MacVlan} instance which represents a L2 host information.
     */
    private final MacVlan  host;

    /**
     * A {@link SalPort} instance corresponding to a switch port to which the
     * host is connected.
     */
    private final SalPort  port;

    /**
     * Construct a new instance.
     *
     * @param mvlan  A {@link MacVlan} instance.
     *               Specifying {@code null} results in undefined behavior.
     * @param sport  A {@link SalPort} instance corresponding to a switch port
     *               to which the host is connected.
     *               Specifying {@code null} results in undefined behavior.
     */
    public L2Host(MacVlan mvlan, SalPort sport) {
        host = mvlan;
        port = sport;
    }

    /**
     * Construct a new instance.
     *
     * @param mac    A MAC address.
     * @param vlan   VLAN ID.
     * @param sport  A {@link SalPort} instance corresponding to a switch port
     *               to which the host is connected.
     *               Specifying {@code null} results in undefined behavior.
     */
    public L2Host(MacAddress mac, int vlan, SalPort sport) {
        this(new MacVlan(mac, vlan), sport);
    }

    /**
     * Construct a new instance.
     *
     * @param mac    MAC address of the host.
     * @param vlan   VLAN ID.
     * @param nc     A {@link NodeConnector} instance corresponding to a
     *               switch port to which the host is connected.
     *               Specifying {@code null} results in undefined behavior.
     */
    public L2Host(byte[] mac, short vlan, NodeConnector nc) {
        this(new MacVlan(mac, vlan), SalPort.create(nc));
    }

    /**
     * Construct a new instance.
     *
     * @param mac    A long integer value which represents a MAC address.
     * @param vlan   VLAN ID.
     * @param nc     A {@link NodeConnector} instance corresponding to a
     *               switch port to which the host is connected.
     *               Specifying {@code null} results in undefined behavior.
     */
    public L2Host(long mac, short vlan, NodeConnector nc) {
        this(new MacVlan(mac, vlan), SalPort.create(nc));
    }

    /**
     * Construct a new instance without specifying MAC address.
     *
     * @param vlan   VLAN ID.
     * @param nc     A {@link NodeConnector} instance corresponding to a
     *               switch port to which the host is connected.
     *               Specifying {@code null} results in undefined behavior.
     */
    public L2Host(short vlan, NodeConnector nc) {
        this(new MacVlan(MacVlan.UNDEFINED, vlan), SalPort.create(nc));
    }

    /**
     * Return a {@link MacVlan} instance which represents a L2 host.
     *
     * <p>
     *   Note that returned {@link MacVlan} instance keeps
     *   {@link MacVlan#UNDEFINED} as MAC address if this instance does not
     *   specify MAC address.
     * </p>
     *
     * @return  A {@link MacVlan} instance.
     */
    public MacVlan getHost() {
        return host;
    }

    /**
     * Return a {@link SalPort} instance corresponding to a switch port.
     *
     * @return  A {@link SalPort} instance corresponding to a switch port to
     *          which the host is connected.
     */
    public SalPort getPort() {
        return port;
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
        if (!(o instanceof L2Host)) {
            return false;
        }

        L2Host l2h = (L2Host)o;
        return (host.equals(l2h.host) && port.equals(l2h.port));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return host.hashCode() ^ port.hashCode();
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("L2Host[host={");
        host.appendContents(builder);
        builder.append("},port=").append(port.toString()).append(']');

        return builder.toString();
    }
}
