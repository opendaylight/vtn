/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.net.InetAddress;
import java.util.Set;
import java.util.HashSet;

import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.util.EtherAddress;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * {@code MacTableEntry} class represents a table entry in the MAC address
 * table.
 */
public class MacTableEntry implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 1443599896969099856L;

    /**
     * The maximum number of IP address probe request.
     */
    private static final int  MAX_IP_PROBE = 10;

    /**
     * A unique identifier for this entry in the cluster.
     */
    private MacTableEntryId  entryId;

    /**
     * A node connector associated with the switch port where the MAC address
     * is detected.
     */
    private final NodeConnector  port;

    /**
     * VLAN ID.
     */
    private final short vlan;

    /**
     * Set of IP addresses.
     */
    private Set<InetAddress>  ipAddresses = new HashSet<InetAddress>();

    /**
     * {@code true} is set if this entry is used.
     *
     * <p>
     *   Note that this field never affects object identity, and it is never
     *   serialized. Below methods always ignore this field.
     * </p>
     * <ul>
     *   <li>{@link #equals(Object)}</li>
     *   <li>{@link #hashCode()}</li>
     *   <li>{@link #writeObject(ObjectOutputStream)}</li>
     * </ul>
     */
    private transient boolean  used = true;

    /**
     * The number of IP address probe request to be sent.
     *
     * <p>
     *   Note that this field never affects object identity, and it is never
     *   serialized. Below methods always ignore this field.
     * </p>
     * <ul>
     *   <li>{@link #equals(Object)}</li>
     *   <li>{@link #hashCode()}</li>
     *   <li>{@link #writeObject(ObjectOutputStream)}</li>
     * </ul>
     */
    private transient int  probeCount;

    /**
     * Construct a new MAC address table entry.
     *
     * @param path    Path to a virtual node which maps the MAC address.
     * @param mac     A long value which represents the MAC address.
     * @param port    A node connector associated with the MAC address.
     *                Specifying {@code null} results in undefined behavior.
     * @param vlan    VLAN ID. Zero means no VLAN ID was found.
     * @param ipaddr  An {@code InetAddress} object which indicates an
     *                IP address assigned to the MAC address.
     *                {@code null} can be specified.
     */
    public MacTableEntry(VBridgePath path, long mac, NodeConnector port,
                         short vlan, InetAddress ipaddr) {
        this(new MacTableEntryId(path, mac), port, vlan, ipaddr);
    }

    /**
     * Construct a new MAC address table entry.
     *
     * @param id      Identifier of this entry.
     *                Specifying {@code null} results in undefined behavior.
     * @param port    A node connector associated with the MAC address.
     *                Specifying {@code null} results in undefined behavior.
     * @param vlan    VLAN ID. Zero means no VLAN ID was found.
     * @param ipaddr  An {@code InetAddress} object which indicates an
     *                IP address assigned to the MAC address.
     *                {@code null} can be specified.
     */
    public MacTableEntry(MacTableEntryId id, NodeConnector port, short vlan,
                         InetAddress ipaddr) {
        this.entryId = id;
        this.port = port;
        this.vlan = vlan;
        if (ipaddr != null) {
            ipAddresses.add(ipaddr);
            probeCount = MAX_IP_PROBE;
        }
    }

    /**
     * Return an identifier of this entry.
     *
     * @return  An identifier of this entry in the cluster.
     */
    public MacTableEntryId getEntryId() {
        return entryId;
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
     * Return a VLAN ID.
     *
     * @return  A VLAN ID. Zero is returned if this MAC address does not belong
     *          to any VLAN.
     */
    public short getVlan() {
        return vlan;
    }

    /**
     * Add an IP address to this entry.
     *
     * @param ipaddr  An {@code InetAddress} object to be added.
     * @return  {@code true} is returned only if the given address was actually
     *          added.
     */
    public synchronized boolean addInetAddress(InetAddress ipaddr) {
        probeCount = MAX_IP_PROBE;
        return ipAddresses.add(ipaddr);
    }

    /**
     * Set a set of IP addresses to this entry.
     *
     * @param ipaddrs  A set of {@code InetAddress} objects.
     *                 Specifying {@code null} results in undefined behavior.
     */
    public synchronized void setInetAddresses(Set<InetAddress> ipaddrs) {
        ipAddresses = ipaddrs;
    }

    /**
     * Return a set of IP addresses assigned to this entry.
     *
     * @return  A set of IP addresses.
     */
    public synchronized Set<InetAddress> getInetAddresses() {
        return new HashSet<InetAddress>(ipAddresses);
    }

    /**
     * Test and clear the used flag.
     *
     * @return  Previous value of the used flag.
     */
    public synchronized boolean clearUsed() {
        // Get and clear reference flag.
        boolean old = this.used;
        this.used = false;
        return old;
    }

    /**
     * Return the current used flag of this entry.
     *
     * @return  Current value of the used flag.
     */
    public synchronized boolean getUsed() {
        return used;
    }

    /**
     * Set the used flag of this entry.
     */
    public synchronized void setUsed() {
        this.used = true;
    }

    /**
     * Determine whether a probe request for IP address should be sent or not,
     * IP address should be sent or not.
     *
     * @return  {@code true} is returned if a probe request for IP address
     *          should be sent. Otherwise {@code false}.
     */
    public synchronized boolean isProbeNeeded() {
        int count = probeCount;
        boolean ret;
        if (count < MAX_IP_PROBE) {
            probeCount = count + 1;
            ret = true;
        } else {
            ret = false;
        }

        return ret;
    }

    /**
     * Create a MAC address entry object.
     *
     * @return  A {@link MacAddressEntry} object.
     * @throws VTNException
     *    Failed to create MAC address entry.
     */
    public MacAddressEntry getEntry() throws VTNException {
        try {
            byte[] mac = EtherAddress.toBytes(entryId.getMacAddress());
            EthernetAddress ethAddr = new EthernetAddress(mac);
            return new MacAddressEntry(ethAddr, vlan, port,
                                       getInetAddresses());
        } catch (Exception e) {
            throw new VTNException("Unable to create MAC address etnry", e);
        }
    }

    /**
     * Return the long value which represents the MAC address.
     *
     * @return  The long value which represents the MAC address.
     */
    public long getMacAddress() {
        return entryId.getMacAddress();
    }

    /**
     * Assign another entry ID to this entry.
     *
     * @return  A new entry ID.
     */
    public MacTableEntryId reassignEntryId() {
        // No need to synchronize this method because this method is called
        // before this entry is put to the cluster cache.
        VBridgePath path = entryId.getMapPath();
        long mac = entryId.getMacAddress();
        MacTableEntryId newId = new MacTableEntryId(path, mac);
        entryId = newId;

        return newId;
    }

    /**
     * Determine whether the host corresponding to this MAC address table
     * entry has moved or not.
     *
     * @param port   A {@link NodeConnector} instance expected to be configured
     *               in this entry.
     * @param vlan   A VLAN ID expected to be configured in this entry.
     * @param mpath  A {@link VBridgePath} instance expected configured in
     *               this entry.
     * @return  {@code true} is returned if the host corresponding to this
     *          instance has moved. Otherwise {@code false} is returned.
     */
    public boolean hasMoved(NodeConnector port, short vlan, VBridgePath mpath) {
        return !(vlan == this.vlan && port.equals(this.port) &&
                 mpath.equals(entryId.getMapPath()));
    }

    /**
     * Read data from the given input stream and deserialize.
     *
     * @param in  An input stream.
     * @throws IOException
     *    An I/O error occurred.
     * @throws ClassNotFoundException
     *    At least one necessary class was not found.
     */
    @SuppressWarnings("unused")
    private void readObject(ObjectInputStream in)
        throws IOException, ClassNotFoundException {
        // Read serialized fields.
        // Note that the monitor of this instance does not need to be acquired
        // here because this instance is not yet visible.
        in.defaultReadObject();

        // Reset "used" to initial value.
        used = true;

        // Disable IP address probe request.
        probeCount = MAX_IP_PROBE;
    }

    /**
     * Serialize this object and write it to the given output stream.
     *
     * @param out  An output stream.
     * @throws IOException
     *    An I/O error occurred.
     */
    @SuppressWarnings("unused")
    private synchronized void writeObject(ObjectOutputStream out)
        throws IOException {
        out.defaultWriteObject();
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
        if (!(o instanceof MacTableEntry)) {
            return false;
        }

        MacTableEntry tent = (MacTableEntry)o;
        if (!entryId.equals(tent.entryId) || !port.equals(tent.port) ||
            vlan != tent.vlan) {
            return false;
        }

        // Compare copies of IP address sets in order to avoid deadlock.
        Set<InetAddress> addrs = getInetAddresses();
        Set<InetAddress> anotherAddrs = tent.getInetAddresses();
        return addrs.equals(anotherAddrs);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = entryId.hashCode() ^ port.hashCode() ^ (int)vlan;

        synchronized (this) {
            h ^= ipAddresses.hashCode();
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
        StringBuilder builder = new StringBuilder("MacTableEntry[");
        builder.append("id=").append(entryId).
            append(",port=").append(port.toString()).
            append(",vlan=").append((int)vlan).
            append(",ipaddr={");

        String sep = "";
        synchronized (this) {
            for (InetAddress ipaddr: ipAddresses) {
                builder.append(sep).append(ipaddr.getHostAddress());
                sep = ",";
            }
        }

        return builder.append("}]").toString();
    }
}
