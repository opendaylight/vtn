/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;
import java.net.InetAddress;
import java.util.HashSet;
import java.util.Set;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;

/**
 * {@code MacAddressEntry} class describes information about the MAC address
 * learned inside
 * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
 *
 * <p>
 *   The VTN Manager passes the MAC address information to other components
 *   by passing {@code MacAddressEntry} object.
 * </p>
 *
 * @see  <a href="package-summary.html#macTable">MAC address table</a>
 */
public class MacAddressEntry implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -9111260127680923458L;

    /**
     * MAC address.
     */
    private final DataLinkAddress  address;

    /**
     * VLAN ID.
     */
    private final short vlan;

    /**
     * Node connector associated with the switch port where the MAC address
     * is detected.
     */
    private final NodeConnector  nodeConnector;

    /**
     * Set of IP addresses found in the ethernet frame.
     */
    private final Set<InetAddress>  inetAddresses = new HashSet<InetAddress>();

    /**
     * Construct a new MAC address entry which represents the MAC address
     * information.
     *
     * @param addr     A {@link DataLinkAddress} object which represents the
     *                 MAC address.
     *                 Specifying {@code null} results in undefined behavior.
     * @param vlan     The VLAN ID configured in the ethernet frame where the
     *                 MAC address is detected.
     *                 Specify <strong>0</strong> if VLAN tag is not present.
     * @param nc       A {@link NodeConnector} object corresponding to the
     *                 physical switch port where MAC address is detected.
     *                 Specifying {@code null} results in undefined behavior.
     * @param ipaddrs
     *   A set of {@link InetAddress} objects which represents IP address
     *   configured in the ethernet frame where the MAC address is detected.
     *   <ul>
     *     <li>
     *       If multiple IP addresses corresponding to the MAC address are
     *       detected, then specify the set that contains all the detected
     *       IP addresses.
     *     </li>
     *     <li>
     *       If no IP address is detected, specify {@code null} or an empty
     *       set.
     *     </li>
     *     <li>
     *       Specifying a set which contains {@code null} results in undefined
     *       behavior.
     *     </li>
     *   </ul>
     */
    public MacAddressEntry(DataLinkAddress addr, short vlan, NodeConnector nc,
                           Set<InetAddress> ipaddrs) {
        address = addr;
        this.vlan = vlan;
        nodeConnector = nc;
        if (ipaddrs != null) {
            inetAddresses.addAll(ipaddrs);
        }
    }

    /**
     * Return the {@link DataLinkAddress} object which represents learned MAC
     * address.
     *
     * @return  The {@link DataLinkAddress} object which represents the MAC
     *          address.
     */
    public DataLinkAddress getAddress() {
        return address;
    }

    /**
     * Return the VLAN ID configured in the ethernet frame where the MAC
     * address is detected.
     *
     * @return  The VLAN ID configured in the ethernet frame where the MAC
     *          address is detected. <strong>0</strong> is returned if no
     *          VLAN tag is configured in the frame.
     */
    public short getVlan() {
        return vlan;
    }

    /**
     * Return the {@link NodeConnector} object corresponding to the physical
     * switch port where the MAC address is detected.
     *
     * @return  The {@link NodeConnector} object corresponding to the physical
     *          switch port where the MAC address is detected.
     */
    public NodeConnector getNodeConnector() {
        return nodeConnector;
    }

    /**
     * Return a set of IP addresses configured in the ethernet frame where
     * the MAC address is detected.
     *
     * @return
     *   A set of IP addresses configured in the ethernet frame where the
     *   MAC address is detected.
     *   <ul>
     *     <li>
     *       If multiple IP addresses corresponding to the MAC address are
     *       detected, a set of IP addresses that contains all the detected
     *       IP addresses is returned.
     *     </li>
     *     <li>
     *       An empty set is returned if no IP address is detected.
     *     </li>
     *   </ul>
     */
    public Set<InetAddress> getInetAddresses() {
        return new HashSet<InetAddress>(inetAddresses);
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code MacAddressEntry} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         {@link DataLinkAddress} object which represents the MAC
     *         address.
     *       </li>
     *       <li>
     *         {@link NodeConnector} object corresponding to the physical
     *         switch port.
     *       </li>
     *       <li>VLAN ID.</li>
     *       <li>A set of IP addresses  associated with the MAC address.</li>
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
        if (!(o instanceof MacAddressEntry)) {
            return false;
        }

        MacAddressEntry mac = (MacAddressEntry)o;
        return (vlan == mac.vlan && address.equals(mac.address) &&
                nodeConnector.equals(mac.nodeConnector) &&
                inetAddresses.equals(mac.inetAddresses));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = address.hashCode() + vlan ^ nodeConnector.hashCode() ^
            inetAddresses.hashCode();

        return h;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("MacAddressEntry[");
        builder.append("address=").append(address.toString()).
            append(",vlan=").append((int)vlan).append(",connector=").
            append(nodeConnector.toString()).
            append(",ipaddr={");

        char sep = 0;
        for (InetAddress ip: inetAddresses) {
            if (sep != 0) {
                builder.append(sep);
            }
            builder.append(ip.getHostAddress());
            sep = ',';
        }

        return builder.append("}]").toString();
    }
}
