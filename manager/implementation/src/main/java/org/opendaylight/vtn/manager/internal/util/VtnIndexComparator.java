/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import java.io.Serializable;
import java.util.Comparator;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnIndex;

/**
 * Comparator for {@link VtnIndex}.
 */
public final class VtnIndexComparator
    implements Comparator<VtnIndex>, Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7994884032297191982L;

    /**
     * Compare the given {@link VtnIndex} instances.
     *
     * @param vi1  The first object to be compared.
     * @param vi2  The second object to be compared.
     * @return    A negative integer, zero, or a positive integer as the
     *            first argument is less than, equal to, or greater than
     *            the second.
     */
    @Override
    public int compare(VtnIndex vi1, VtnIndex vi2) {
        Integer i1 = vi1.getIndex();
        Integer i2 = vi2.getIndex();

        return i1.compareTo(i2);
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        return (o != null && VtnIndexComparator.class.equals(o.getClass()));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return VtnIndexComparator.class.hashCode();
    }
}
