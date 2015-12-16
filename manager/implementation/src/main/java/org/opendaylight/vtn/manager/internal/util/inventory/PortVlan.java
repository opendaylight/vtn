/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.inventory;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import org.opendaylight.vtn.manager.internal.util.VlanDescParser;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

/**
 * {@code PortVlan} class represents a pair of the physical switch port and
 * the VLAN ID.
 */
public final class PortVlan {
    /**
     * A {@link SalPort} instance associated with the physical switch port.
     */
    private final SalPort  port;

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
     * @param sport  A {@link SalPort} instance associated with the physical
     *               switch port.
     *               Specifying {@code null} results in undefined behavior.
     * @param vid    VLAN ID.
     */
    public PortVlan(SalPort sport, int vid) {
        port = sport;
        vlanId = vid;
    }

    /**
     * Construct a new instance from the given string.
     *
     * @param value  A string concatenated a node-connector-id and a VLAN ID
     *               with "@".
     * @throws RpcException
     *    An invalid value is specified to {@code value}.
     */
    public PortVlan(String value) throws RpcException {
        VlanDescParser parser = new VlanDescParser(value, "port-vlan");
        port = SalPort.create(parser.getIdentifier());
        if (port == null) {
            throw RpcException.getBadArgumentException(
                "Invalid node-connector-id in port-vlan: " + value);
        }

        vlanId = parser.getVlanId();
        stringCache = value;
    }

    /**
     * Return a {@link SalPort} instance associated with the physical switch
     * port.
     *
     * @return  A {@link SalPort} instance.
     */
    public SalPort getPort() {
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
            h = PortVlan.class.hashCode();
            h = h * HASH_PRIME + port.hashCode();
            h = h * HASH_PRIME + vlanId;
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
            value = port.toString() + VlanDescParser.SEPARATOR + vlanId;
            stringCache = value;
        }

        return value;
    }
}
