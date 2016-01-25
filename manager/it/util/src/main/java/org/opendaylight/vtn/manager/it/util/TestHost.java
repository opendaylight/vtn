/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util;

import java.util.Objects;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.Ip4Network;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.it.util.inventory.MacVlan;
import org.opendaylight.vtn.manager.it.util.vnode.mac.MacEntry;

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
     * An {@link EtherAddress} which represents the MAC address.
     */
    private final EtherAddress  macAddress;

    /**
     * IP address.
     */
    private final IpNetwork  inetAddress;

    /**
     * MD-SAL node connector identifier which specifies the switch port
     * to which this host is connected.
     */
    private final String  portIdentifier;

    /**
     * The port name.
     */
    private final String  portName;

    /**
     * VLAN ID.
     */
    private final int  vlanId;

    /**
     * Construct a new instance.
     *
     * @param index  A integer value used to create host addresses.
     * @param pid    Port identifier.
     * @param pname  The name of the port.
     * @param vid    VLAN ID.
     */
    public TestHost(int index, String pid, String pname, int vid) {
        byte[] maddr = MAC_ADDRESS_BASE.clone();
        maddr[maddr.length - 1] = (byte)index;
        macAddress = new EtherAddress(maddr);

        byte[] addr = IPV4_ADDRESS_BASE.clone();
        addr[addr.length - 1] = (byte)index;
        inetAddress = new Ip4Network(addr);
        portIdentifier = pid;
        portName = pname;
        vlanId = vid;
    }

    /**
     * Construct a new instance.
     *
     * @param mac    MAC address of the host.
     * @param vid    VLAN ID.
     * @param pid    Port identifier.
     * @param pname  The name of the port.
     * @param iaddr  IP address of the host.
     */
    public TestHost(EtherAddress mac, int vid, String pid, String pname,
                    IpNetwork iaddr) {
        macAddress = mac;
        vlanId = vid;
        portIdentifier = pid;
        portName = pname;
        inetAddress = iaddr;
    }

    /**
     * Copy the given host with changing VLAN ID.
     *
     * @param host  A host to be copied.
     * @param vid   VLAN ID.
     */
    public TestHost(TestHost host, int vid) {
        macAddress = host.macAddress;
        inetAddress = host.inetAddress;
        portIdentifier = host.portIdentifier;
        portName = host.portName;
        vlanId = vid;
    }

    /**
     * Return MAC address of this host.
     *
     * @return  MAC address.
         */
    public byte[] getMacAddress() {
        return macAddress.getBytes();
    }

    /**
     * Return an {@link EtherAddress} instance which represents the MAC address
     * of this host.
     *
     * @return  An {@link EtherAddress} instance.
     */
    public EtherAddress getEtherAddress() {
        return macAddress;
    }

    /**
     * Return IP address of this host.
     *
     * @return  An {@link IpNetwork} instance.
     */
    public IpNetwork getInetAddress() {
        return inetAddress;
    }

    /**
     * Return raw IP address of this host.
     *
     * @return  A byte array which represents the IP address of this host.
     */
    public byte[] getRawInetAddress() {
        return inetAddress.getBytes();
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
     * Return the port name.
     *
     * @return  The port name.
     */
    public String getPortName() {
        return portName;
    }

    /**
     * Return VLAN ID of this host..
     *
     * @return  VLAN ID.
     */
    public int getVlanId() {
        return vlanId;
    }

    /**
     * Return a {@link MacVlan} instance that represents this host.
     *
     * @return  A {@link MacVlan} instance.
     */
    public MacVlan getMacVlan() {
        return new MacVlan(macAddress, vlanId);
    }

    /**
     * Return a {@link MacEntry} instance which represents this host.
     *
     * @return  A {@link MacEntry} instance.
     */
    public MacEntry getMacEntry() {
        return getMacEntry(true);
    }

    /**
     * Return a {@link MacEntry} instance which represents this host.
     *
     * @param useIp  If {@code true}, set IP address of this host into
     *               a {@link MacEntry} instance.
     *               If {@code false}, IP address of this host is ignored.
     * @return  A {@link MacEntry} instance.
     */
    public MacEntry getMacEntry(boolean useIp) {
        IpNetwork ip = (useIp) ? inetAddress : null;
        return new MacEntry(macAddress, vlanId, portIdentifier, portName, ip);
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
            TestHost th = (TestHost)o;
            ret = (macAddress.equals(th.macAddress) && vlanId == th.vlanId);
            if (ret) {
                ret = (portIdentifier.equals(th.portIdentifier) &&
                       portName.equals(th.portName) &&
                       inetAddress.equals(th.inetAddress));
            }
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
        return Objects.hash(macAddress, inetAddress, portIdentifier,
                            portName, vlanId);
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
            append(", inet=").append(inetAddress).
            append(", port-id=").append(portIdentifier).
            append(", port-name=").append(portName).
            append(", vlan=").append(vlanId).append(']');
        return builder.toString();
    }
}
