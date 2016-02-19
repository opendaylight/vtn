/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Objects;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VlanHostDesc;

/**
 * An implementation of vlan-host-desc that bypasses value check.
 */
public final class TestVlanHostDesc extends VlanHostDesc {
    /**
     * The VLAN-host descriptor.
     */
    private final String  descriptor;

    /**
     * Construct an empty instance.
     */
    public TestVlanHostDesc() {
        this(null);
    }

    /**
     * Construct a new instance.
     *
     * @param value  The VLAN-host descriptor.
     */
    public TestVlanHostDesc(String value) {
        super("00:00:00:00:00:00@0");
        descriptor = value;
    }

    // VlanHostDesc

    /**
     * Return the VLAN-host descriptor.
     *
     * @return  The VLAN-host descriptor.
     */
    @Override
    public String getValue() {
        return descriptor;
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
            TestVlanHostDesc vhd = (TestVlanHostDesc)o;
            ret = Objects.equals(descriptor, vhd.descriptor);
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
        return Objects.hash(getClass(), descriptor);
    }
}
