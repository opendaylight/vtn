/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Objects;

import org.opendaylight.yang.gen.v1.urn.opendaylight.l2.types.rev130827.EtherType;

/**
 * An implementation of {@link EtherType} that bypasses value check.
 */
public final class TestEtherType extends EtherType {
    /**
     * An Ethernet type.
     */
    private final Long  etherType;

    /**
     * Construct an empty instance.
     */
    public TestEtherType() {
        this(null);
    }

    /**
     * Construct a new instance.
     *
     * @param type  An Ethernet type.
     */
    public TestEtherType(Long type) {
        super(1L);
        etherType = type;
    }

    // EtherType

    /**
     * Return an Ethernet type.
     *
     * @return  An Ethernet type.
     */
    @Override
    public Long getValue() {
        return etherType;
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
            TestEtherType et = (TestEtherType)o;
            ret = Objects.equals(etherType, et.etherType);
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
        return Objects.hash(getClass(), etherType);
    }
}
