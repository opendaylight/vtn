/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Objects;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.yang.types.rev100924.MacAddress;

/**
 * An implementation of {@link MacAddress} that bypasses value check.
 */
public final class TestMacAddress extends MacAddress {
    /**
     * The MAC address.
     */
    private final String  macAddress;

    /**
     * Construct an empty instance.
     */
    public TestMacAddress() {
        this(null);
    }

    /**
     * Construct a new instance.
     *
     * @param mac  The MAC address.
     */
    public TestMacAddress(String mac) {
        super("00:00:00:00:00:00");
        macAddress = mac;
    }

    // MacAddress

    /**
     * Return the MAC address.
     *
     * @return  The MAC address.
     */
    @Override
    public String getValue() {
        return macAddress;
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
            TestMacAddress mac = (TestMacAddress)o;
            ret = Objects.equals(macAddress, mac.macAddress);
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
        return Objects.hash(getClass(), macAddress);
    }
}
