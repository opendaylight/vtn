/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.inventory;

import java.util.Objects;

import org.opendaylight.vtn.manager.it.util.VlanDescParser;

/**
 * {@code PortVlan} class represents a pair of the physical switch port and
 * the VLAN ID.
 */
public final class PortVlan {
    /**
     * The port identifier.
     */
    private final String  port;

    /**
     * VLAN ID.
     */
    private final int  vlanId;

    /**
     * Cache for a string representation.
     */
    private String  stringCache;

    /**
     * Cache for a hash code.
     */
    private int  hash;

    /**
     * Construct a new instance.
     *
     * @param pid  The MD-SAL port identifier.
     * @param vid  VLAN ID.
     */
    public PortVlan(String pid, int vid) {
        port = pid;
        vlanId = vid;
    }

    /**
     * Construct a new instance from the given string.
     *
     * @param value  A string concatenated a node-connector-id and a VLAN ID
     *               with "@".
     */
    public PortVlan(String value) {
        VlanDescParser parser = new VlanDescParser(value, "port-vlan");
        port = parser.getIdentifier();
        vlanId = parser.getVlanId();
        stringCache = value;
    }

    /**
     * Return the MD-SAL port identifier.
     *
     * @return  The MD-SAL port identifier.
     */
    public String getPort() {
        return port;
    }

    /**
     * Return the VLAN ID.
     *
     * @return  VLAN ID.
     */
    public int getVlanId() {
        return vlanId;
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        boolean ret = (o == this);
        if (!ret && o != null && getClass().equals(o.getClass())) {
            PortVlan pv = (PortVlan)o;
            ret = (vlanId == pv.vlanId && port.equals(pv.port));
        }

        return ret;
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = hash;
        if (h == 0) {
            h = Objects.hash(getClass(), port, vlanId);
            hash = h;
        }

        return h;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        String value = stringCache;
        if (value == null) {
            value = port + VlanDescParser.SEPARATOR + vlanId;
            stringCache = value;
        }

        return value;
    }
}
