/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.net.InetAddress;
import java.util.Set;
import java.util.HashSet;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * {@code IpAddressSet} class describes a set of IP addresses.
 *
 * <p>
 *   An instance of {@code IpAddressSet} contains zero or more
 *   {@link IpAddress} instances.
 * </p>
 * <ul>
 *   <li>
 *     Order of {@link IpAddress} instances in a {@code IpAddressSet} instance
 *     is unspecified.
 *   </li>
 *   <li>
 *     An {@code IpAddressSet} instance contains no duplicate
 *     {@link IpAddress} insntances.
 *   </li>
 * </ul>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"inetAddress": [
 * &nbsp;&nbsp;&nbsp;&nbsp;{"address": "192.168.10.1"},
 * &nbsp;&nbsp;&nbsp;&nbsp;{"address": "10.1.2.3"}
 * &nbsp;&nbsp;]
 * }</pre>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "inetAddresses")
@XmlAccessorType(XmlAccessType.NONE)
public class IpAddressSet {
    /**
     * A set of {@link IpAddress} instances which represents a set of
     * IP addresses.
     */
    @XmlElement(name = "inetAddress")
    private final Set<IpAddress>  ipAddress = new HashSet<IpAddress>();

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private IpAddressSet() {
    }

    /**
     * Construct a new set of IP address.
     *
     * @param ipaddrs  A set of {@code InetAddress} object.
     * @throws NullPointerException
     *    {@code ipaddrs} is {@code null}.
     */
    public IpAddressSet(Set<InetAddress> ipaddrs) {
        for (InetAddress iaddr: ipaddrs) {
            ipAddress.add(new IpAddress(iaddr));
        }
    }

    /**
     * Return a set of {@link IpAddress} objects.
     *
     * @return  A set of {@link IpAddress} objects.
     */
    Set<IpAddress> getAddresses() {
        return ipAddress;
    }

    /**
     * Return the number of IP addresses in this object.
     *
     * @return  The number of IP addresses.
     */
    int getLength() {
        return ipAddress.size();
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
        if (!(o instanceof IpAddressSet)) {
            return false;
        }

        IpAddressSet iaddrs = (IpAddressSet)o;
        return ipAddress.equals(iaddrs.ipAddress);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return ipAddress.hashCode();
    }
}
