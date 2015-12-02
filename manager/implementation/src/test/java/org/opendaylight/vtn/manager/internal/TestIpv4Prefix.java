/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Objects;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Ipv4Prefix;

/**
 * An implementation of {@link Ipv4Prefix} that bypasses value check.
 */
public final class TestIpv4Prefix extends Ipv4Prefix {
    /**
     * An IPv4 address and prefix.
     */
    private final String  ipAddress;

    /**
     * Construct an empty instance.
     */
    public TestIpv4Prefix() {
        this(null);
    }

    /**
     * Construct a new instance.
     *
     * @param ip  An IPv4 address and prefix in CIDR form.
     */
    public TestIpv4Prefix(String ip) {
        super("127.0.0.1/32");
        ipAddress = ip;
    }

    // Ipv4Prefix

    /**
     * Return an IPv4 address and prefix.
     *
     * @return  An IPv4 address and prefix.
     */
    @Override
    public String getValue() {
        return ipAddress;
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
            TestIpv4Prefix ip = (TestIpv4Prefix)o;
            ret = Objects.equals(ipAddress, ip.ipAddress);
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
        return Objects.hash(getClass(), ipAddress);
    }
}
