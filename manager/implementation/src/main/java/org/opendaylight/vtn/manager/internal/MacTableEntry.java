/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.net.InetAddress;
import java.util.HashSet;

import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.VTNException;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;
import org.opendaylight.controller.sal.utils.NetUtils;

/**
 * {@code MacTableEntry} class represents a table entry in the MAC address
 * table.
 *
 * <p>
 *   Note that this class is not synchronized.
 * </p>
 */
public class MacTableEntry {
    /**
     * A node connector associated with the switch port where the MAC address
     * is detected.
     */
    private NodeConnector  port;

    /**
     * VLAN ID.
     */
    private short vlan;

    /**
     * Set of IP addresses.
     */
    private HashSet<InetAddress>  ipAddresses = new HashSet<InetAddress>();

    /**
     * {@code true} is set if this entry is used.
     */
    private boolean  used = true;

    /**
     * Construct a new MAC address table entry.
     *
     * @param port    A node connector associated with the MAC address.
     *                Specifying {@code null} results in undefined behavior.
     * @param vlan    VLAN ID. Zero means no VLAN ID was found.
     * @param ipaddr  An {@code InetAddress} object which indicates an
     *                IP address assigned to the MAC address.
     *                {@code null} can be specified.
     */
    MacTableEntry(NodeConnector port, short vlan, InetAddress ipaddr) {
        this.port = port;
        this.vlan = vlan;
        if (ipaddr != null) {
            ipAddresses.add(ipaddr);
        }
    }

    /**
     * Return a node connector associated with the physical switch port.
     *
     * @return  A node connector.
     */
    public NodeConnector getPort() {
        return port;
    }

    /**
     * Set a node connector associated with the physical switch port.
     *
     * @param newport  A node connector.
     */
    public void setPort(NodeConnector newport) {
        port = newport;
    }

    /**
     * Return a VLAN ID.
     *
     * @return  A VLAN ID. Zero is returned if this MAC address does not belong
     *          to any VLAN.
     */
    public short getVlan() {
        return vlan;
    }

    /**
     * Set a VLAN ID.
     *
     * @param newvlan  A VLAN ID.
     */
    public void setVlan(short newvlan) {
        vlan = newvlan;
    }

    /**
     * Add an IP address to this entry.
     *
     * @paran ipaddr  An {@code InetAddress} object to be added.
     * @return  {@code true} is returned only if the given address was actually
     *          added.
     */
    public boolean addInetAddress(InetAddress ipaddr) {
        return ipAddresses.add(ipaddr);
    }

    /**
     * Test and clear the used flag.
     *
     * @return  Previous value of the used flag.
     */
    public boolean clearUsed() {
        // Get and clear reference flag.
        boolean old = this.used;
        this.used = false;
        return old;
    }

    /**
     * Set the used flag of this entry.
     */
    public void setUsed() {
        this.used = true;
    }

    /**
     * Create a MAC address entry object.
     *
     * @param mac  A long value which represents a MAC address.
     * @return  A {@link MacAddressEntry} object.
     */
    public MacAddressEntry getEntry(long mac) throws VTNException {
        try {
            byte[] macAddr = NetUtils.longToByteArray6(mac);
            EthernetAddress ethAddr = new EthernetAddress(macAddr);
            return new MacAddressEntry(ethAddr, this.vlan, this.port,
                                       ipAddresses);
        } catch (Exception e) {
            throw new VTNException("Unable to create MAC address etnry", e);
        }
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("MacTableEntry[");
        builder.append("port=").append(port.toString()).
            append(",vlan=").append((int)vlan).
            append(",ipaddr={");

        char sep = 0;
        for (InetAddress ipaddr: ipAddresses) {
            if (sep == 0) {
                sep = ',';
            } else {
                builder.append(sep);
            }
            builder.append(ipaddr.getHostAddress());
        }

        return builder.append("}]").toString();
    }
}
