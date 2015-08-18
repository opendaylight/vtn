/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.net.InetAddress;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * {@code IpAddress} class describes an IP address.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"address": "192.168.10.1"
 * }</pre>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "inetAddress")
@XmlAccessorType(XmlAccessType.NONE)
public class IpAddress {
    /**
     * String representation of IP address.
     */
    @XmlAttribute(required = true)
    private String  address;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private IpAddress() {
    }

    /**
     * Construct a new IP address.
     *
     * @param ipaddr  An {@code InetAddress} object.
     * @throws NullPointerException
     *    {@code ipaddr} is {@code null}.
     */
    public IpAddress(InetAddress ipaddr) {
        address = ipaddr.getHostAddress();
    }

    /**
     * Return a string representation of IP address.
     *
     * @return  A string representation of IP address.
     */
    String getAddress() {
        return address;
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
        if (!(o instanceof IpAddress)) {
            return false;
        }

        IpAddress iaddr = (IpAddress)o;
        if (address == null) {
            return (iaddr.address == null);
        }

        return address.equals(iaddr.address);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 1;
        if (address != null) {
            h += address.hashCode();
        }

        return h;
    }
}
