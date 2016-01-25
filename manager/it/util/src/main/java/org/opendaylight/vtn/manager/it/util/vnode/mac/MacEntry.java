/*
 * Copyright (c) 2015, 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.it.util.vnode.mac;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;

import org.opendaylight.vtn.manager.util.EtherAddress;
import org.opendaylight.vtn.manager.util.IpNetwork;

import org.opendaylight.vtn.manager.it.ofmock.OfMockUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.mac.rev150907.VtnMacEntry;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.IpAddress;

/**
 * {@code MacEntry} describes a MAC address information in the MAC address
 * table.
 */
public final class MacEntry implements Cloneable {
    /**
     * The MAC address.
     */
    private final EtherAddress  macAddress;

    /**
     * The VLAN ID.
     */
    private final int  vlanId;

    /**
     * The identifier for the switch where the MAC address is detected.
     */
    private String  nodeId;

    /**
     * The port ID that specifies the switch port where the MAC address is
     * detected.
     */
    private String  portId;

    /**
     * The port name that specifies the switch port where the MAC address is
     * detected.
     */
    private String  portName;

    /**
     * A set of IP addresses found in the Ethernet frame.
     */
    private Set<IpNetwork>  ipAddresses;

    /**
     * The MD-SAL port identifier that specifies the switch port where the
     * MAC address is detected.
     */
    private String  portIdentifier;

    /**
     * Construct a new instance.
     *
     * @param mac    The MAC address.
     * @param vid    The VLAN ID.
     * @param nid    The identifier for the switch where the MAC address is
     *               detected.
     * @param pid    The port ID that specifies the switch port where the
     *               MAC address is detected.
     * @param pname  THe port name that specifies the switch port where the
     *               MAC address is detected.
     * @param ip     An IP address found in the Ethernet frame.
     */
    public MacEntry(EtherAddress mac, int vid, String nid, String pid,
                    String pname, IpNetwork ip) {
        macAddress = mac;
        vlanId = vid;
        nodeId = nid;
        portId = pid;
        portName = pname;
        ipAddresses = (ip == null)
            ? Collections.<IpNetwork>emptySet()
            : Collections.singleton(ip);
    }

    /**
     * Construct a new instance.
     *
     * @param mac    The MAC address.
     * @param vid    The VLAN ID.
     * @param pid    The MD-SAL port identifier that specifies the switch port
     *               where the MAC address is detected.
     * @param pname  THe port name that specifies the switch port where the
     *               MAC address is detected.
     * @param ip     An IP address found in the Ethernet frame.
     */
    public MacEntry(EtherAddress mac, int vid, String pid, String pname,
                    IpNetwork ip) {
        macAddress = mac;
        vlanId = vid;
        nodeId = OfMockUtils.getNodeIdentifier(pid);
        portId = OfMockUtils.getPortId(pid);
        portIdentifier = pid;
        portName = pname;
        ipAddresses = (ip == null)
            ? Collections.<IpNetwork>emptySet()
            : Collections.singleton(ip);
    }

    /**
     * Construct a new instance.
     *
     * @param vment  A {@link VtnMacEntry} instance.
     */
    public MacEntry(VtnMacEntry vment) {
        macAddress = EtherAddress.create(vment.getMacAddress());
        vlanId = vment.getVlanId().intValue();
        nodeId = vment.getNode().getValue();
        portId = vment.getPortId();
        portName = vment.getPortName();
        ipAddresses = toIpNetworkSet(vment.getIpAddresses());
    }

    /**
     * Return the MAC address.
     *
     * @return  An {@link EtherAddress} instance that represents the MAC
     *          adddress.
     */
    public EtherAddress getMacAddress() {
        return macAddress;
    }

    /**
     * Return the VLAN ID.
     *
     * @return  The VLAN ID.
     */
    public int getVlanId() {
        return vlanId;
    }

    /**
     * Return the identifier for the switch where the MAC address is detected.
     * detected.
     *
     * @return  The MD-SAL node identifier.
     */
    public String getNodeIdentifier() {
        return nodeId;
    }

    /**
     * Return the port ID that specifies the switch port where the MAC
     * address is detected. detected.
     *
     * @return  A string representation of the switch port ID.
     */
    public String getPortId() {
        return portId;
    }

    /**
     * Return the port name that specifies the switch port where the MAC
     * address is detected. detected.
     *
     * @return  The port name.
     */
    public String getPortName() {
        return portName;
    }

    /**
     * Return the MD-SAL port identifier that specifies the switch port
     * where the MAC address is detected. detected.
     *
     * @return  The MD-SAL port identifier.
     */
    public String getPortIdentifier() {
        String pid = portIdentifier;
        if (pid == null) {
            pid = OfMockUtils.getPortIdentifier(nodeId, portId);
            portIdentifier = pid;
        }

        return pid;
    }

    /**
     * Create a copy of this instance, and set the given switch port to the
     * copied instance.
     *
     * @param pid    The MD-SAL port identifier.
     * @param pname  The name of the switch port.
     * @return  A copy of this instance with the given switch port information.
     */
    public MacEntry setPortIdentifier(String pid, String pname) {
        MacEntry ment = clone();
        ment.nodeId = OfMockUtils.getNodeIdentifier(pid);
        ment.portId = OfMockUtils.getPortId(pid);
        ment.portIdentifier = pid;
        ment.portName = pname;
        return ment;
    }

    /**
     * Return a set of IP addresses found in the Ethernet frame.
     *
     * @return  A set of {@link IpNetwork} instances.
     */
    public Set<IpNetwork> getIpAddresses() {
        return ipAddresses;
    }

    /**
     * Create a copy of this instance, and add the given IP address to the
     * copied instance.
     *
     * @param ipn  The IP address to be added.
     * @return  A copy of this instance with adding the given IP address.
     */
    public MacEntry addIpAddress(IpNetwork ipn) {
        MacEntry ment = clone();
        if (ipAddresses.isEmpty()) {
            ment.ipAddresses = Collections.singleton(ipn);
        } else {
            Set<IpNetwork> set = new HashSet<>(ipAddresses);
            set.add(ipn);
            ment.ipAddresses = Collections.unmodifiableSet(set);
        }

        return ment;
    }

    /**
     * Convert the given list of MD-SAL IP addresses into a set of
     * {@link IpNetwork} instances.
     *
     * @param list  A list of MD-SAL IP addresses.
     * @return  A set of {@link IpNetwork} instances.
     */
    private Set<IpNetwork>  toIpNetworkSet(List<IpAddress> list) {
        Set<IpNetwork> set;
        if (list == null || list.isEmpty()) {
            set = Collections.<IpNetwork>emptySet();
        } else {
            set = new HashSet<>();
            for (IpAddress ip: list) {
                IpNetwork ipn = IpNetwork.create(ip);
                assertNotNull(ipn);
                assertEquals(true, set.add(ipn));
            }
            set = Collections.unmodifiableSet(set);
        }

        return set;
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
            MacEntry ment = (MacEntry)o;
            ret = (macAddress.equals(ment.macAddress) &&
                   vlanId == ment.vlanId &&
                   ipAddresses.equals(ment.ipAddresses));
            if (ret) {
                ret = (nodeId.equals(ment.nodeId) &&
                       portId.equals(ment.portId) &&
                       portName.equals(ment.portName));
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
        return Objects.hash(getClass(), macAddress, vlanId, nodeId, portId,
                            portName, ipAddresses);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        return "mac-entry[addr=" + macAddress + ", vlan=" + vlanId +
            ", node-id=" + nodeId + ", port-id=" + portId +
            ", port-name-" + portName + ", ip=" + ipAddresses + "]";
    }

    /**
     * Create a shallow copy of this instance.
     *
     * @return  A shallow copy of this instance.
     */
    @Override
    public MacEntry clone() {
        try {
            return (MacEntry)super.clone();
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
