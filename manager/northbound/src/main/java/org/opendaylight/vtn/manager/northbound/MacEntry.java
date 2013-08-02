/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.net.InetAddress;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.SwitchPort;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * {@code MacEntry} class provides JAXB mapping for MAC address table entry
 * leaned by the virtual L2 bridge.
 */
@XmlRootElement(name = "macentry")
@XmlAccessorType(XmlAccessType.NONE)
public class MacEntry {
    /**
     * String representation of MAC address.
     */
    @XmlAttribute
    private String  address;

    /**
     * VLAN ID.
     */
    @XmlAttribute
    private short  vlan;

    /**
     * Node associated with the swtich where the MAC address is detected.
     */
    @XmlElement(required = true)
    private Node  node;

    /**
     * Identifier of node connector associated with the switch port where the
     * MAC address is detected.
     */
    @XmlElement(required = true)
    private SwitchPort  port;

    /**
     * An array of IP addresses found in the Ethernet frame.
     */
    private IpAddressArray  inetAddresses;

    /**
     * Private constructor used for JAXB mapping.
     */
    private MacEntry() {
    }

    /**
     * Construct a new MAC address entry.
     *
     * @param entry  MAC address entry.
     * @throws NullPointerException
     *    {@code entry} is {@code null}.
     */
    public MacEntry(MacAddressEntry entry) {
        DataLinkAddress dladdr = entry.getAddress();
        if (dladdr instanceof EthernetAddress) {
            EthernetAddress ethaddr = (EthernetAddress)dladdr;
            this.address = ethaddr.getMacAddress();
        } else {
            this.address = dladdr.toString();
        }

        NodeConnector nc = entry.getNodeConnector();
        String type = nc.getType();
        String id = nc.getNodeConnectorIDString();
        this.port = new SwitchPort(type, id);
        this.node = nc.getNode();
        this.vlan = entry.getVlan();

        InetAddress[] ipaddrs = entry.getInetAddresses();
        if (ipaddrs.length != 0) {
            this.inetAddresses = new IpAddressArray(ipaddrs);
        }
    }

    /**
     * Return a string representation of MAC address.
     *
     * @return  A string representation of MAC address.
     */
    String getAddress() {
        return address;
    }

    /**
     * Return the VLAN ID associated with this MAC address entry.
     *
     * @return  VLAN ID. Zero means no VLAN tag.
     */
    short getVlan() {
        return vlan;
    }

    /**
     * Return a node associated with the switch where the MAC address is
     * detected.
     *
     * @return  A node object.
     */
    Node getNode() {
        return node;
    }

    /**
     * Return a {@code SwitchPort} object which represents the switch port
     * where the MAC address is detected.
     *
     * @return  A {@code SwitchPort} object.
     */
    SwitchPort getPort() {
        return port;
    }

    /**
     * Return an {@link IpAddressArray} object which keeps IP addresses
     * associated with this MAC address entry.
     *
     * @return  An {@link IpAddressArray} object.
     *          {@code null} is returned if no IP address is associated
     *          with this MAC address entry.
     */
    @XmlElement(name = "inetAddresses")
    IpAddressArray getInetAddresses() {
        return inetAddresses;
    }

    /**
     * Set an {@link IpAddressArray} object.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param ipaddrs  An {@link IpAddressArr} object.
     */
    private void setInetAddresses(IpAddressArray ipaddrs) {
        if (ipaddrs != null && ipaddrs.getLength() != 0) {
            inetAddresses = ipaddrs;
        } else {
            inetAddresses = null;
        }
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
        if (!(o instanceof MacEntry)) {
            return false;
        }

        MacEntry entry = (MacEntry)o;
        if (vlan != entry.vlan) {
            return false;
        }
        if (!address.equals(entry.address)) {
            return false;
        }
        if (!node.equals(entry.node)) {
            return false;
        }
        if (!port.equals(entry.port)) {
            return false;
        }

        if (inetAddresses == null) {
            return (entry.inetAddresses == null);
        }

        return inetAddresses.equals(entry.inetAddresses);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 31 + address.hashCode() + node.hashCode() + port.hashCode() +
            vlan;
        if (inetAddresses != null) {
            h += inetAddresses.hashCode();
        }

        return h;
    }
}
