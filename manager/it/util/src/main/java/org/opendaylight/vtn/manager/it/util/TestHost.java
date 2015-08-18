/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util;

import java.net.InetAddress;
import java.util.Collections;
import java.util.Objects;
import java.util.Set;

import org.opendaylight.vtn.manager.EthernetHost;
import org.opendaylight.vtn.manager.MacAddressEntry;

import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * {@code TestHost} describes a host information used for integration test.
 */
public final class TestHost {
    /**
     * Base MAC address for a host.
     */
    private static final byte[] MAC_ADDRESS_BASE = {
        (byte)0x00, (byte)0x11, (byte)0x22,
        (byte)0x33, (byte)0x44, (byte)0x00,
    };

    /**
     * Base IPv4 address for a host.
     */
    private static final byte[] IPV4_ADDRESS_BASE = {
        (byte)192, (byte)168, (byte)100, (byte)0,
    };

    /**
     * A {@link EthernetAddress} which represents the MAC address.
     */
    private final EthernetAddress  macAddress;

    /**
     * IP address.
     */
    private final InetAddress  inetAddress;

    /**
     * MD-SAL node connector identifier which specifies the switch port
     * to which this host is connected.
     */
    private final String  portIdentifier;

    /**
     * VLAN ID.
     */
    private final short  vlan;

    /**
     * Construct a new instance.
     *
     * @param index  A integer value used to create host addresses.
     * @param pid    Port identifier.
     * @param vid    VLAN ID.
     * @throws Exception  An error occurred.
     */
    public TestHost(int index, String pid, short vid) throws Exception {
        byte[] maddr = MAC_ADDRESS_BASE.clone();
        maddr[maddr.length - 1] = (byte)index;
        macAddress = new EthernetAddress(maddr);

        byte[] addr = IPV4_ADDRESS_BASE.clone();
        addr[addr.length - 1] = (byte)index;
        inetAddress = InetAddress.getByAddress(addr);
        portIdentifier = pid;
        vlan = vid;
    }

    /**
     * Copy the given host with changing VLAN ID.
     *
     * @param host  A host to be copied.
     * @param vid  VLAN ID.
     */
    public TestHost(TestHost host, short vid) {
        macAddress = host.macAddress;
        inetAddress = host.inetAddress;
        portIdentifier = host.portIdentifier;
        vlan = vid;
    }

    /**
     * Return MAC address of this host.
     *
     * @return  MAC address.
         */
    public byte[] getMacAddress() {
        return macAddress.getValue();
    }

    /**
     * Return an {@link EthernetAddress} instance which represents the
     * MAC address of this host.
     *
     * @return  An {@link EthernetAddress} instance.
     */
    public EthernetAddress getEthernetAddress() {
        return macAddress;
    }

    /**
     * Return IP address of this host.
     *
     * @return  An {@link InetAddress} instance.
     */
    public InetAddress getInetAddress() {
        return inetAddress;
    }

    /**
     * Return raw IP address of this host.
     *
     * @return  A byte array which represents the IP address of this host.
     */
    public byte[] getRawInetAddress() {
        return inetAddress.getAddress();
    }

    /**
     * Return the port identifier.
     *
     * @return  The port identifier.
     */
    public String getPortIdentifier() {
        return portIdentifier;
    }

    /**
     * Return VLAN ID of this host..
     *
     * @return  VLAN ID.
     */
    public short getVlan() {
        return vlan;
    }

    /**
     * Return an {@link EthernetHost} instance which represents this host.
     *
     * @return  An {@link EthernetHost} instance.
     */
    public EthernetHost getEthernetHost() {
        return new EthernetHost(macAddress, vlan);
    }

    /**
     * Return an {@link MacAddressEntry} instance which represents this host.
     *
     * @return  A {@link MacAddressEntry} instance.
     */
    public MacAddressEntry getMacAddressEntry() {
        return getMacAddressEntry(true);
    }

    /**
     * Return an {@link MacAddressEntry} instance which represents this host.
     *
     * @param useIp  If {@code true}, set IP address of this host into
     *               an {@link MacAddressEntry} instance.
     *               If {@code false}, IP address of this host is ignored.
     * @return  A {@link MacAddressEntry} instance.
     */
    public MacAddressEntry getMacAddressEntry(boolean useIp) {
        NodeConnector nc = TestBase.toAdNodeConnector(portIdentifier);
        Set<InetAddress> ipaddrs = (useIp)
            ? Collections.singleton(inetAddress)
            : null;
        return new MacAddressEntry(macAddress, vlan, nc, ipaddrs);
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
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        TestHost th = (TestHost)o;

        return (macAddress.equals(th.macAddress) &&
                inetAddress.equals(th.inetAddress) &&
                portIdentifier.equals(th.portIdentifier) && vlan == th.vlan);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(macAddress, inetAddress, portIdentifier, vlan);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("TestHost[mac=").
            append(macAddress).
            append(",inet=").append(inetAddress).
            append(",port=").append(portIdentifier).
            append(",vlan=").append(vlan).append(']');
        return builder.toString();
    }
}
