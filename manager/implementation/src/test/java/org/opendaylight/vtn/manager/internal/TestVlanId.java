/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Objects;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanId;

/**
 * An implementation of {@link VlanId} that bypasses value check.
 */
public final class TestVlanId extends VlanId {
    /**
     * A VLAN ID.
     */
    private final Integer  vlanId;

    /**
     * Construct an empty instance.
     */
    public TestVlanId() {
        this(null);
    }

    /**
     * Construct a new instance.
     *
     * @param vid  A VLAN ID.
     */
    public TestVlanId(Integer vid) {
        super(0);
        vlanId = vid;
    }

    // VlanId

    /**
     * Return a VLAN ID.
     *
     * @return  A VLAN ID.
     */
    @Override
    public Integer getValue() {
        return vlanId;
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
            TestVlanId vid = (TestVlanId)o;
            ret = Objects.equals(vlanId, vid.vlanId);
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
        return Objects.hash(getClass(), vlanId);
    }
}
