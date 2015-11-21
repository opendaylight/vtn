/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode.mac;

import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntryBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpAddress;

/**
 * {@code MacEntry} describes a MAC address learned in the MAC address table
 * associated with vBridge.
 */
public abstract class MacEntry {
    /**
     * The maximum number of IP address probe request.
     */
    public static final int  MAX_IP_PROBE = 10;

    /**
     * The MAC address.
     */
    private final EtherAddress  etherAddress;

    /**
     * The physical switch port where the MAC address was detected.
     */
    private final SalPort  port;

    /**
     * The VLAN ID configured in the MAC address table entry.
     */
    private final int  vlanId;

    /**
     * The number of IP address probe request.
     */
    private int  ipProbe;

    /**
     * Construct a new instance.
     *
     * @param eaddr  An {@link EtherAddress} instance that indicates the
     *               MAC address.
     * @param sport  A {@link SalPort} instance that specifies the switch port
     *               where the MAC address was detected.
     * @param vid    The VLAN ID.
     */
    protected MacEntry(EtherAddress eaddr, SalPort sport, int vid) {
        this(eaddr, sport, vid, 0);
    }

    /**
     * Construct a new instance.
     *
     * @param eaddr  An {@link EtherAddress} instance that indicates the
     *               MAC address.
     * @param sport  A {@link SalPort} instance that specifies the switch port
     *               where the MAC address was detected.
     * @param vid    The VLAN ID.
     * @param probe  The number of IP address probe request.
     */
    protected MacEntry(EtherAddress eaddr, SalPort sport, int vid, int probe) {
        etherAddress = eaddr;
        port = sport;
        vlanId = vid;
        ipProbe = probe;
    }

    /**
     * Return the MAC address.
     *
     * @return  An {@link EtherAddress} instance.
     */
    public final EtherAddress getEtherAddress() {
        return etherAddress;
    }

    /**
     * Return the physical switch port where the MAC address was detected.
     *
     * @return  A {@link SalPort} instance that specifies the physical switch
     *          port.
     */
    public final SalPort getPort() {
        return port;
    }

    /**
     * Return the VLAN ID configured in the MAC address table entry.
     *
     * @return  The VLAN ID. Zero implies that the MAC address was detected
     *          in an untagged Ethernet frame.
     */
    public final int getVlanId() {
        return vlanId;
    }

    /**
     * Determine whether the host corresponding to this MAC address table
     * entry has moved or not.
     *
     * @param sport  A {@link SalPort} instance expected to be configured
     *               in this entry.
     * @param vid    A VLAN ID expected to be configured in this entry.
     * @param mpath  A {@link VNodeIdentifier} instance expected to be
     *               configured in this entry.
     * @return  {@code true} if the host corresponding to this instance has
     *          moved. {@code false} otherwise.
     */
    public final boolean hasMoved(SalPort sport, int vid,
                                  VNodeIdentifier<?> mpath) {
        return !(vid == vlanId && port.equalsPort(sport) &&
                 getMapPath().equals(mpath.toString()));
    }

    /**
     * Determine whether a probe request for IP address should be sent or not.
     *
     * @return  {@code true} if a probe request for IP address should be sent.
     *          {@code false} otherwise.
     */
    public final boolean needIpProbe() {
        boolean ret = getIpNetworkSet().isEmpty();
        if (ret) {
            int count = ipProbe;
            if (count < MAX_IP_PROBE) {
                // Bump the probe count.
                ipProbe = count + 1;
                setDirty();
            } else {
                // Too many probes.
                ret = false;
            }
        }

        return ret;
    }

    /**
     * Set values in this instance into the given builder instance.
     *
     * @param builder  A {@link MacTableEntryBuilder} instance.
     * @return  {@code builder} is always returned.
     */
    protected final MacTableEntryBuilder set(MacTableEntryBuilder builder) {
        return builder.setIpAddresses(getIpAddresses()).
            setIpProbeCount(ipProbe).
            setUsed(true);
    }

    /**
     * Associate the given IP address with this MAC address table entry.
     *
     * @param ip  An {@link IpNetwork} instance.
     */
    public abstract void addIpAddress(IpNetwork ip);

    /**
     * Return a list of {@link IpAddress} instances associated with this
     * MAC address table entry.
     *
     * @return  A list of {@link IpAddress} instances or {@code null}.
     */
    public abstract List<IpAddress> getIpAddresses();

    /**
     * Return a set of {@link IpNetwork} instances associated with this
     * MAC address table entry.
     *
     * @return  A set of {@link IpNetwork} instances.
     *          Note that this method must not return {@code null}.
     */
    public abstract Set<IpNetwork> getIpNetworkSet();

    /**
     * Return a string representation of {@link VNodeIdentifier} instance that
     * specifies the virtual mapping corresponding to this entry.
     *
     * @return  A string that represents the identifier for the virtual
     *          mapping.
     */
    public abstract String getMapPath();

    /**
     * Turn the dirty flag on so that this entry is put into the MD-SAL
     * datastore.
     */
    public abstract void setDirty();

    /**
     * Return the MAC address table entry to be put into the MD-SAL datastore.
     *
     * @return  A {@link MacTableEntry} instance if this entry needs to be put
     *          into the MD-SAL datastore. {@code null} otherwise.
     */
    public abstract MacTableEntry getNewEntry();

    // Object

    /**
     * Return a string representation of this instance.
     *
     * @return  A string representation of this instance.
     */
    @Override
    public final String toString() {
        StringBuilder builder = new StringBuilder("mac-table-entry[mac=").
            append(etherAddress.getText()).
            append(", port=").append(port).
            append(", vid=").append(vlanId).
            append(", mapPath=").append(getMapPath());

        Set<IpNetwork> ipSet = getIpNetworkSet();
        if (!ipSet.isEmpty()) {
            builder.append(", ipaddr=").append(ipSet);
        }

        return builder.append(']').toString();
    }
}
