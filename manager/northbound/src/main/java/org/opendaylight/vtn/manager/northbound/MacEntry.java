/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.net.InetAddress;
import java.util.Objects;
import java.util.Set;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.MacAddressEntry;
import org.opendaylight.vtn.manager.SwitchPort;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.packet.address.DataLinkAddress;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * {@code MacEntry} class describes information about the MAC address
 * learned inside vBridge.
 *
 * <p>
 *   This class is used to return MAC address information inside the MAC
 *   address table to REST client.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"address": "01:02:03:04:05:06",
 * &nbsp;&nbsp;"vlan": "0",
 * &nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:11:22:33"
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"port": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;"id": "1"
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"inetAddresses": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"inetAddress": [
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{"address": "192.168.10.1"}
 * &nbsp;&nbsp;&nbsp;&nbsp;]
 * &nbsp;&nbsp;}
 * }</pre>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "macentry")
@XmlAccessorType(XmlAccessType.NONE)
public class MacEntry {
    /**
     * A string representation of a learned MAC address.
     *
     * <ul>
     *   <li>
     *     A MAC address is represented by hexadecimal notation with
     *     {@code ':'} inserted between octets.
     *     (e.g. {@code "11:22:33:aa:bb:cc"})
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private String  address;

    /**
     * A VLAN ID detected inside ethernet frame where MAC address was detected.
     *
     * <ul>
     *   <li><strong>0</strong> implies that VLAN tag was not detected.</li>
     * </ul>
     */
    @XmlAttribute
    private short  vlan;

    /**
     * {@link Node} information corresponding to the physical switch where
     * MAC address was detected.
     */
    @XmlElement(required = true)
    private Node  node;

    /**
     * {@link NodeConnector} information corresponding to the physical switch
     * port where MAC address was detected.
     */
    @XmlElement(required = true)
    private SwitchPort  port;

    /**
     * IP address information detected inside ethernet frame where MAC address
     * was detected.
     *
     * <ul>
     *   <li>
     *     If multiple IP addresses corresponding to MAC address are detected,
     *     this element contains all of them.
     *   </li>
     *   <li>This element will be omitted if no IP address is detected.</li>
     * </ul>
     */
    private IpAddressSet  inetAddresses;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
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

        Set<InetAddress> ipaddrs = entry.getInetAddresses();
        if (ipaddrs.size() != 0) {
            this.inetAddresses = new IpAddressSet(ipaddrs);
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
     * where the MAC address was detected.
     *
     * @return  A {@code SwitchPort} object.
     */
    SwitchPort getPort() {
        return port;
    }

    /**
     * A set of IP addresses found in the ethernet frame where the MAC address
     * was detected.
     *
     * <ul>
     *   <li>
     *     If multiple IP address corresponding to the MAC address are
     *     detected, all detected IP addresses are configured.
     *   </li>
     *   <li>
     *     This element is omitted if no IP address is detected.
     *   </li>
     * </ul>
     *
     * @return  An {@link IpAddressSet} object.
     *          {@code null} is returned if no IP address is associated
     *          with this MAC address entry.
     */
    @XmlElement(name = "inetAddresses")
    public IpAddressSet getInetAddresses() {
        return inetAddresses;
    }

    /**
     * Set an {@link IpAddressSet} object.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param ipaddrs  An {@link IpAddressSet} object.
     */
    @SuppressWarnings("unused")
    private void setInetAddresses(IpAddressSet ipaddrs) {
        if (ipaddrs != null && ipaddrs.getLength() != 0) {
            inetAddresses = ipaddrs;
        } else {
            inetAddresses = null;
        }
    }

    /**
     * Determine whether the given object contains the same inventory
     * information with this object.
     *
     * @param entry  A {@link MacEntry} instance to be compared.
     * @return  {@code true} only if the given object contains the same
     *          inventory information.
     */
    private boolean equalsInventory(MacEntry entry) {
        return (node.equals(entry.node) && port.equals(entry.port));
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
        return (vlan == entry.vlan && address.equals(entry.address) &&
                equalsInventory(entry) &&
                Objects.equals(inetAddresses, entry.inetAddresses));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = address.hashCode() ^ node.hashCode() ^ port.hashCode() + vlan;
        if (inetAddresses != null) {
            h += inetAddresses.hashCode();
        }

        return h;
    }
}
