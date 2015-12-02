/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Objects;

import org.opendaylight.yang.gen.v1.urn.ietf.params.xml.ns.yang.ietf.inet.types.rev100924.Dscp;

/**
 * An implementation of {@link Dscp} that bypasses value check.
 */
public final class TestDscp extends Dscp {
    /**
     * A DSCP value.
     */
    private final Short  dscp;

    /**
     * Construct an empty instance.
     */
    public TestDscp() {
        this(null);
    }

    /**
     * Construct a new instance.
     *
     * @param d  A DSCP value.
     */
    public TestDscp(Short d) {
        super((short)0);
        dscp = d;
    }

    // Dscp

    /**
     * Return a DSCP value.
     *
     * @return  A DSCP value.
     */
    @Override
    public Short getValue() {
        return dscp;
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
            TestDscp d = (TestDscp)o;
            ret = Objects.equals(dscp, d.dscp);
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
        return Objects.hash(getClass(), dscp);
    }
}
