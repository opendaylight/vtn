/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode.mac;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.inventory.SalPort;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntry;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.vtn.mac.table.entry.MacTableEntryBuilder;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpAddress;

/**
 * {@code CurrentMacEntry} describes a new MAC address table entry obtained
 * from the MD-SAL datastore.
 */
public final class CurrentMacEntry extends MacEntry {
    /**
     * A {@link MacTableEntry} instance obtained from the MD-SAL datastore.
     */
    private final MacTableEntry  theEntry;

    /**
     * A set of IP addresses associated with this entry.
     */
    private Set<IpNetwork>  ipAddressSet;

    /**
     * A boolean value that determines whether this entry needs to be put into
     * the MD-SAL datastore.
     */
    private boolean  dirty;

    /**
     * Construct a new instance.
     *
     * @param eaddr  An {@link EtherAddress} instance that indicates the
     *               MAC address.
     * @param mtent  A {@link MacTableEntry} instance obtained from the
     *               MD-SAL datastore.
     */
    public CurrentMacEntry(EtherAddress eaddr, MacTableEntry mtent) {
        super(eaddr, SalPort.create(mtent), mtent.getVlanId().intValue(),
              mtent.getIpProbeCount().intValue());
        theEntry = mtent;
    }

    // MacEntry

    /**
     * {@inheritDoc}
     */
    @Override
    public void addIpAddress(IpNetwork ip) {
        if (getIpNetworkSet().add(ip)) {
            dirty = true;
        }
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public List<IpAddress> getIpAddresses() {
        List<IpAddress> list;
        Set<IpNetwork> ipSet = ipAddressSet;
        if (MiscUtils.isEmpty(ipSet)) {
            // No IP address is added by addIpAddress(IpNetwork).
            list = theEntry.getIpAddresses();
        } else {
            list = new ArrayList<>(ipSet.size());
            for (IpNetwork ipn: ipSet) {
                list.add(ipn.getIpAddress());
            }
        }
        return list;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Set<IpNetwork> getIpNetworkSet() {
        Set<IpNetwork> ipSet = ipAddressSet;
        if (ipSet == null) {
            ipSet = new HashSet<>();
            ipAddressSet = ipSet;

            List<IpAddress> ipaddrs = theEntry.getIpAddresses();
            if (!MiscUtils.isEmpty(ipaddrs)) {
                for (IpAddress ip: ipaddrs) {
                    ipSet.add(IpNetwork.create(ip));
                }
            }
        }

        return ipSet;
    }

    /**
     * {@inheritDoc}
     */
    public String getMapPath() {
        return theEntry.getEntryData();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void setDirty() {
        dirty = true;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public MacTableEntry getNewEntry() {
        MacTableEntry mtent;
        if (dirty) {
            MacTableEntryBuilder builder = new MacTableEntryBuilder(theEntry);
            mtent = set(builder).build();
        } else {
            mtent = null;
        }

        return mtent;
    }
}
