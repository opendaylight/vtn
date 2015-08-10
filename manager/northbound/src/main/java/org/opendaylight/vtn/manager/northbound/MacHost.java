/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.EthernetHost;

import org.opendaylight.controller.northbound.commons.exception.
    InternalServerErrorException;
import org.opendaylight.controller.sal.packet.address.EthernetAddress;

/**
 * {@code MacHost} class describes a host information specified by a pair of
 * MAC address and VLAN ID.
 *
 * <p>
 *   An instance of this class keeps a MAC address and a VLAN ID, and used
 *   to represent host information in data link layer for REST client.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"address": "00:11:22:33:44:55",
 * &nbsp;&nbsp;"vlan": 10
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "machost")
@XmlAccessorType(XmlAccessType.NONE)
public class MacHost {
    /**
     * A string representation of a MAC address.
     *
     * <ul>
     *   <li>
     *     A MAC address is represented by hexadecimal notation with
     *     {@code ':'} inserted between octets.
     *     (e.g. {@code "11:22:33:aa:bb:cc"})
     *   </li>
     *   <li>
     *     If this attribute is omitted, it will be treated as if all hosts
     *     over the VLAN specified in <strong>vlan</strong> attribute are
     *     specified.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private String  address;

    /**
     * A VLAN ID that specifies the VLAN network.
     *
     * <ul>
     *   <li><strong>0</strong> specifies untagged network.</li>
     *   <li>
     *     If omitted, it will be treated as if <strong>0</strong> is
     *     specified.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private short  vlan;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private MacHost() {
    }

    /**
     * Construct a new host information in Ethernet layer.
     *
     * @param dlhost  A {@link DataLinkHost} instance.
     * @throws InternalServerErrorException
     *    {@code dlhost} is not an instance of {@link EthernetHost}.
     */
    public MacHost(DataLinkHost dlhost) {
        if (dlhost instanceof EthernetHost) {
            EthernetHost ehost = (EthernetHost)dlhost;
            EthernetAddress eaddr = ehost.getAddress();
            if (eaddr != null) {
                address = eaddr.getMacAddress();
            }
            vlan = ehost.getVlan();
        } else {
            // This should never happen.
            throw new InternalServerErrorException(
                "Not an Ethernet host: " + dlhost);
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
     * Return the VLAN ID associated with this host information.
     *
     * @return  VLAN ID. Zero means that the host specified by this instance
     *          is on untagged network.
     */
    short getVlan() {
        return vlan;
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
        if (!(o instanceof MacHost)) {
            return false;
        }

        MacHost host = (MacHost)o;
        if (vlan != host.vlan) {
            return false;
        }

        if (address == null) {
            return (host.address == null);
        }

        return address.equals(host.address);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = (int)vlan;
        if (address != null) {
            h ^= address.hashCode();
        }

        return h;
    }
}
