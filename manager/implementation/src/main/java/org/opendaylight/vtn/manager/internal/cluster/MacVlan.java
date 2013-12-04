/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;

import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * {@code MacVlan} class represents a pair of MAC address and VLAN ID.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class MacVlan implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -4031121202176886188L;

    /**
     * MAC address.
     */
    private final long  macAddress;

    /**
     * VLAN ID.
     */
    private final short  vlan;

    /**
     * Construct a new instance.
     *
     * @param mac   A long value which represents a MAC address.
     * @param vlan  VLAN ID.
     */
    public MacVlan(long mac, short vlan) {
        macAddress = mac;
        this.vlan = vlan;
    }

    /**
     * Construct a new instance.
     *
     * @param mac   A byte array which represents a MAC address.
     * @param vlan  VLAN ID.
     */
    public MacVlan(byte[] mac, short vlan) {
        this(NetUtils.byteArray6ToLong(mac), vlan);
    }

    /**
     * Return a long value which represents a MAC address.
     *
     * @return  A long value which represent a MAC address.
     */
    public long getMacAddress() {
        return macAddress;
    }

    /**
     * Return a VLAN ID.
     *
     * @return  VLAN ID.
     */
    public short getVlan() {
        return vlan;
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
        if (!(o instanceof MacVlan)) {
            return false;
        }

        MacVlan mvlan = (MacVlan)o;
        return (macAddress == mvlan.macAddress && vlan == mvlan.vlan);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return VTNManagerImpl.hashCode(macAddress) ^ vlan;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("MacVlan[");
        builder.append("addr=").
            append(VTNManagerImpl.formatMacAddress(macAddress)).
            append(",vlan=").append((int)vlan).append(']');

        return builder.toString();
    }
}
