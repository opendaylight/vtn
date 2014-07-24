/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.internal.cluster.MacVlan;

import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * An instance of {@code L2Host} class specifies the location of layer 2 host.
 */
public final class L2Host {
    /**
     * A {@link MacVlan} instance which represents a L2 host information.
     */
    private final MacVlan  host;

    /**
     * A {@link NodeConnector} instance corresponding to a switch port
     * to which the host is connected.
     */
    private final NodeConnector  port;

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
        host = new MacVlan(mac, vlan);
        port = nc;
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
        host = new MacVlan(mac, vlan);
        port = nc;
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
        host = new MacVlan(MacVlan.UNDEFINED, vlan);
        port = nc;
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
     * Return a {@link NodeConnector} instance corresponding to a switch port.
     *
     * @return  A {@link NodeConnector} instance corresponding to a switch
     *          port to which the host is connected.
     */
    public NodeConnector getPort() {
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
