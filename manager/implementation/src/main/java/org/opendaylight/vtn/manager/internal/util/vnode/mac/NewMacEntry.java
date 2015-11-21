/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode.mac;

import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntryBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpAddress;

/**
 * {@code NewMacEntry} describes a new MAC address table entry to be put into
 * the MD-SAL datastore.
 */
public final class NewMacEntry extends MacEntry {
    /**
     * The name of the switch port where the MAC address was detected.
     */
    private String  portName;

    /**
     * An IP address associated with this entry.
     */
    private final IpNetwork  ipAddress;

    /**
     * A string that specifies the virtual mapping that maps the MAC address.
     */
    private final String  mapPath;

    /**
     * Construct a new instance to be put into the MD-SAL datastore.
     *
     * @param eaddr  An {@link EtherAddress} instance that indicates the
     *               MAC address.
     * @param sport  A {@link SalPort} instance that specifies the switch port
     *               where the MAC address was detected.
     * @param pname  The name of the port specified by {@code sport}.
     * @param vid    The VLAN ID.
     * @param ip     An IP address detected inside Ethernet frame where the
     *               MAC address was detected.
     * @param ident  A {@link VNodeIdentifier} that specifies the virtual
     *               mapping that maps the MAC address.
     */
    public NewMacEntry(EtherAddress eaddr, SalPort sport, String pname,
                       int vid, IpNetwork ip, VNodeIdentifier<?> ident) {
        super(eaddr, sport, vid);
        ipAddress = ip;
        portName = pname;
        mapPath = ident.toString();
    }

    // MacEntry

    /**
     * This method should never be called because the IP address is already
     * specified by the constructor.
     *
     * @param ip  Unused.
     * @throws IllegalStateException  Always thrown.
     */
    @Override
    public void addIpAddress(IpNetwork ip) {
        throw MiscUtils.unexpected();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public List<IpAddress> getIpAddresses() {
        return (ipAddress == null)
            ? null
            : Collections.singletonList(ipAddress.getIpAddress());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<IpNetwork> getIpNetworkSet() {
        return (ipAddress == null)
            ? Collections.<IpNetwork>emptySet()
            : Collections.singleton(ipAddress);
    }

    /**
     * {@inheritDoc}
     */
    public String getMapPath() {
        return mapPath;
    }

    /**
     * This method does nothing because this entry represents a new entry
     * that is always put into the MD-SAL datastore.
     */
    @Override
    public void setDirty() {
    }

    /**
     * Return the MAC address table entry to be put into the MD-SAL datastore.
     *
     * @return  A {@link MacTableEntry} instance.
     */
    @Override
    public MacTableEntry getNewEntry() {
        SalPort sport = getPort();
        MacTableEntryBuilder builder = new MacTableEntryBuilder().
            setMacAddress(getEtherAddress().getMacAddress()).
            setNode(sport.getNodeId()).
            setPortId(String.valueOf(sport.getPortNumber())).
            setPortName(portName).
            setVlanId(getVlanId()).
            setEntryData(mapPath);

        return set(builder).build();
    }
}
