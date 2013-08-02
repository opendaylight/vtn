/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;
import java.net.InetAddress;
import java.util.Arrays;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;

/**
 * {@code MacAddressEntry} class describes a MAC address table entry learned
 * by the virtual L2 bridge.
 */
public class MacAddressEntry implements Serializable {
    private final static long serialVersionUID = -4983331952133216806L;

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
     * IP addresses found in the Ethernet frame.
     */
    private final InetAddress[]  inetAddresses;

    /**
     * Construct a new MAC address entry.
     *
     * @param addr     MAC address.
     *                 Specifying {@code null} results in undefined behavior.
     * @param vlan     VLAN ID associated with the Ethernet frame.
     *                 Zero means no VLAN tag.
     * @param nc       Node connector associated with the switch port where
     *                 the MAC address is detected.
     *                 Specifying {@code null} results in undefined behavior.
     * @param ipaddrs  An array of IP addresses found in the Ethernet frame.
     *                 Specifying {@code null} or an empty array means no IP
     *                 address was found.
     *                 Specifying an array which contains {@code null} results
     *                 in undefined behavior.
     */
    public MacAddressEntry(DataLinkAddress addr, short vlan, NodeConnector nc,
                           InetAddress[] ipaddrs) {
        address = addr;
        this.vlan = vlan;
        nodeConnector = nc;
        if (ipaddrs == null) {
            inetAddresses = new InetAddress[0];
        } else {
            inetAddresses = ipaddrs.clone();
        }
    }

    /**
     * Return the MAC address.
     *
     * @return  MAC address.
     */
    public DataLinkAddress getAddress() {
        return address;
    }

    /**
     * Return the VLAN ID associated with this MAC address entry.
     *
     * @return  VLAN ID. Zero means no VLAN tag.
     */
    public short getVlan() {
        return vlan;
    }

    /**
     * Return a node connector associated with the switch port where the
     * MAC address is detected.
     *
     * @return  A node connector.
     */
    public NodeConnector getNodeConnector() {
        return nodeConnector;
    }

    /**
     * Return an array of IP addresss associated with this MAC address entry.
     *
     * @return  An array of IP addresses. An empty array is returned if no
     *          IP address is associated.
     */
    public InetAddress[] getInetAddresses() {
        return inetAddresses.clone();
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
        if (!(o instanceof MacAddressEntry)) {
            return false;
        }

        MacAddressEntry mac = (MacAddressEntry)o;
        if (vlan != mac.vlan) {
            return false;
        }
        if (!address.equals(mac.address)) {
            return false;
        }
        if (!nodeConnector.equals(mac.nodeConnector)) {
            return false;
        }

        return Arrays.equals(inetAddresses, mac.inetAddresses);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 367 + address.hashCode() + vlan + nodeConnector.hashCode() +
            Arrays.hashCode(inetAddresses);

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
