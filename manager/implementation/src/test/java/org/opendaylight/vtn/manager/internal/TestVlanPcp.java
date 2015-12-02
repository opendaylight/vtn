/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Objects;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.VlanPcp;

/**
 * An implementation of {@link VlanPcp} that bypasses value check.
 */
public final class TestVlanPcp extends VlanPcp {
    /**
     * A VLAN priority.
     */
    private final Short  vlanPcp;

    /**
     * Construct an empty instance.
     */
    public TestVlanPcp() {
        this(null);
    }

    /**
     * Construct a new instance.
     *
     * @param pcp  A VLAN priority.
     */
    public TestVlanPcp(Short pcp) {
        super((short)0);
        vlanPcp = pcp;
    }

    // VlanPcp

    /**
     * Return a VLAN priority.
     *
     * @return  A VLAN priority.
     */
    @Override
    public Short getValue() {
        return vlanPcp;
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
            TestVlanPcp vpcp = (TestVlanPcp)o;
            ret = Objects.equals(vlanPcp, vpcp.vlanPcp);
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
        return Objects.hash(getClass(), vlanPcp);
    }
}
