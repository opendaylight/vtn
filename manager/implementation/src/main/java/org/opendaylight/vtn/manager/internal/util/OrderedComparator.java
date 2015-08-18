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

import org.opendaylight.yang.gen.v1.urn.opendaylight.action.types.rev131112.Ordered;

/**
 * Comparator for {@link Ordered}.
 */
public final class OrderedComparator
    implements Comparator<Ordered>, Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 8641505928821657861L;

    /**
     * Compare the given {@link Ordered} instances.
     *
     * @param o1  The first object to be compared.
     * @param o2  The second object to be compared.
     * @return    A negative integer, zero, or a positive integer as the
     *            first argument is less than, equal to, or greater than
     *            the second.
     */
    @Override
    public int compare(Ordered o1, Ordered o2) {
        Integer i1 = o1.getOrder();
        Integer i2 = o2.getOrder();

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
        return (o != null && OrderedComparator.class.equals(o.getClass()));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return OrderedComparator.class.hashCode();
    }
}
