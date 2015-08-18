/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.controller.sal.packet.address.DataLinkAddress;

/**
 * A pseudo data link address for test.
 */
public class TestDataLink extends DataLinkAddress {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 1L;

    /**
     * The name of this address type.
     */
    private static final String  ADDRESS_NAME = "Test Address";

    /**
     * A string which identifies the address.
     */
    private final String  address;

    /**
     * Construct a new address.
     */
    public TestDataLink() {
        this("");
    }

    /**
     * Construct a new address.
     *
     * @param addr  A string which identifies the address.
     */
    public TestDataLink(String addr) {
        super(ADDRESS_NAME);
        address = (addr == null) ? "" : addr;
    }

    /**
     * Return a clone of this object.
     *
     * @return  A clone of this object.
     */
    @Override
    public TestDataLink clone() {
        return new TestDataLink(new String(address));
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
        if (!(o instanceof TestDataLink)) {
            return false;
        }

        TestDataLink dl = (TestDataLink)o;
        return address.equals(dl.address);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return address.hashCode();
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        return "TestDataLink [" + address + "]";
    }
}

