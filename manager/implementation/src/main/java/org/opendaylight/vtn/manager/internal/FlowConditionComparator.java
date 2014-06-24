/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Comparator;

import org.opendaylight.vtn.manager.flow.cond.FlowCondition;

/**
 * Comparator for {@link FlowCondition} instances, that compares the
 * flow condition name.
 */
public final class FlowConditionComparator
    implements Comparator<FlowCondition> {
    /**
     * Compare the given {@link FlowCondition} instances.
     *
     * @param f1  The first condition to be compared.
     * @param f2  The second condition to be compared.
     * @return    A negative integer, zero, or a positive integer as the
     *            first argument is less than, equal to, or greater than
     *            the second.
     */
    @Override
    public int compare(FlowCondition f1, FlowCondition f2) {
        String name1 = f1.getName();
        String name2 = f2.getName();

        return name1.compareTo(name2);
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        return (o != null &&
                FlowConditionComparator.class.equals(o.getClass()));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return FlowConditionComparator.class.hashCode();
    }
}
