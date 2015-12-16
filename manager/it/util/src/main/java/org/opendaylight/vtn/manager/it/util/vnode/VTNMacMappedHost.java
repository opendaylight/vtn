/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode;

import java.util.Objects;

import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.mapping.mac.rev150907.vtn.mac.map.status.MappedHost;

/**
 * {@code VTNMacMappedHost} describes host information mapped by the
 * MAC mapping.
 */
public final class VTNMacMappedHost {
    /**
     * The MAC address of the host.
     */
    private final EtherAddress  macAddress;

    /**
     * The VLAN ID.
     */
    private final int  vlanId;

    /**
     * The identifier for the switch port where the MAC address is detected.
     */
    private final String  portId;

    /**
     * Construct a new instance.
     *
     * @param mac  The MAC address.
     * @param vid  The VLAN ID.
     * @param pid  The identifier for the switch port where the MAC address
     *             is detected.
     */
    public VTNMacMappedHost(EtherAddress mac, int vid, String pid) {
        macAddress = mac;
        vlanId = vid;
        portId = pid;
    }

    /**
     * Construct a new instance.
     *
     * @param mh  A {@link MappedHost} instance.
     */
    public VTNMacMappedHost(MappedHost mh) {
        macAddress = EtherAddress.create(mh.getMacAddress());
        vlanId = mh.getVlanId().getValue().intValue();
        portId = mh.getPortId().getValue();
    }

    /**
     * Return the MAC address.
     *
     * @return  An {@link EtherAddress} instance that represents the MAC
     *          adddress.
     */
    public EtherAddress getMacAddress() {
        return macAddress;
    }

    /**
     * Return the VLAN ID.
     *
     * @return  The VLAN ID.
     */
    public int getVlanId() {
        return vlanId;
    }

    /**
     * Return the identifier for the switch port where the MAC address is
     * detected.
     *
     * @return  The switch port identifier.
     */
    public String getPortIdentifier() {
        return portId;
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
            VTNMacMappedHost mhost = (VTNMacMappedHost)o;
            ret = (macAddress.equals(mhost.macAddress) &&
                   vlanId == mhost.vlanId &&
                   portId.equals(mhost.portId));
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
        return Objects.hash(getClass(), macAddress, vlanId, portId);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        return "mapped-host[addr=" + macAddress.getText() +
            ", vlan=" + vlanId + ", port=" + portId + "]";
    }
}
