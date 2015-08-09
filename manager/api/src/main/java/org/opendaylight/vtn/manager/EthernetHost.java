/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * {@code EthernetHost} class is used to represent a host information for
 * Ethernet layer.
 *
 * <p>
 *   An instance of this class keeps a pair of MAC address and VLAN ID.
 * </p>
 *
 * @since  Helium
 */
public class EthernetHost extends DataLinkHost {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -5055373331732301357L;

    /**
     * A VLAN ID.
     */
    private final short  vlan;

    /**
     * Construct a new instance without specifying MAC address.
     *
     * @param vlan
     *    A VLAN ID.
     *    <ul>
     *      <li><strong>0</strong> means untagged network.
     *    </ul>
     */
    public EthernetHost(short vlan) {
        this(null, vlan);
    }

    /**
     * Construct a new instance which represents a MAC address on the VLAN
     * specified by the VLAN ID.
     *
     * @param addr
     *    An {@link EthernetAddress} instance which represents a MAC address.
     *    <ul>
     *      <li>
     *        Specifying {@code null} means that the address is not specified.
     *        If {@code null} is specified, it will create an instance that
     *        specifies all hosts present on the VLAN specified by
     *        {@code vlan}.
     *      </li>
     *    </ul>
     * @param vlan
     *    A VLAN ID.
     *    <ul>
     *      <li><strong>0</strong> means untagged network.
     *    </ul>
     */
    public EthernetHost(EthernetAddress addr, short vlan) {
        super(addr);
        this.vlan = vlan;
    }

    /**
     * Return a MAC address configured in this instance.
     *
     * @return  A {@link EthernetAddress} instance which represents a
     *          MAC address. {@code null} is returned if it is not configured
     *          in this instance.
     */
    @Override
    public EthernetAddress getAddress() {
        return (EthernetAddress)super.getAddress();
    }

    /**
     * Return a VLAN ID configured in this instance.
     *
     * @return  A VLAN ID.
     *          Note that <strong>0</strong> is returned if untagged network
     *          is configured in this instance.
     */
    public short getVlan() {
        return vlan;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is an {@code EthernetHost} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         A {@link EthernetAddress} instance which represents a
     *         MAC address.
     *       </li>
     *       <li>A VLAN ID.</li>
     *     </ul>
     *   </li>
     * </ul>
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!super.equals(o)) {
            return false;
        }

        EthernetHost ehost = (EthernetHost)o;
        return (vlan == ehost.vlan);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return (int)vlan ^ super.hashCode();
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("EthernetHost[");
        EthernetAddress eaddr = getAddress();
        if (eaddr != null) {
            builder.append("address=").append(eaddr.getMacAddress()).
                append(',');
        }

        builder.append("vlan=").append((int)vlan).append(']');

        return builder.toString();
    }
}
