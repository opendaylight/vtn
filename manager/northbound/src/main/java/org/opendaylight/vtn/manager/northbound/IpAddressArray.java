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
import java.util.Arrays;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

/**
 * JAXB mapping for an array of {@code InetAddress}.
 */
@XmlRootElement(name = "inetAddresses")
@XmlAccessorType(XmlAccessType.NONE)
public class IpAddressArray {
    /**
     * An array of IP address in text.
     */
    @XmlElement(name = "inetAddress")
    private IpAddress[]  ipAddress;

    /**
     * Private constructor used for JAXB mapping.
     */
    private IpAddressArray() {
        ipAddress = new IpAddress[0];
    }

    /**
     * Construct a new array of IP address.
     *
     * @param ipaddrs  An array of {@code InetAddress} object.
     * @throws NullPointerException
     *    {@code ipaddrs} is {@code null}.
     */
    public IpAddressArray(InetAddress[] ipaddrs) {
        ipAddress = new IpAddress[ipaddrs.length];
        for (int i = 0; i < ipaddrs.length; i++) {
            ipAddress[i] = new IpAddress(ipaddrs[i]);
        }
    }

    /**
     * Return an {@link IpAddress} object at the given index.
     *
     * @param index  An array index.
     * @return  An {@link IpAddress} object.
     * @throws ArrayIndexOutOfBoundsException
     *    An illegal array index is passed to {@code index}.
     */
    IpAddress get(int index) {
        return ipAddress[index];
    }

    /**
     * Return the number of IP addresses in this object.
     *
     * @return  The number of IP addresses.
     */
    int getLength() {
        return ipAddress.length;
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
        if (!(o instanceof IpAddressArray)) {
            return false;
        }

        IpAddressArray iaddrs = (IpAddressArray)o;
        return Arrays.equals(ipAddress, iaddrs.ipAddress);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Arrays.hashCode(ipAddress);
    }
}
