/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import java.io.Serializable;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;

import org.opendaylight.yangtools.yang.binding.InstanceIdentifier;

/**
 * Compares {@link InstanceIdentifier} instances using its target type.
 *
 * <p>
 *   The order of target type can be specified by
 *   {@link #setOrder(Class, int)}. {@link Integer#MAX_VALUE} will be
 *   associated with unknown target type.
 * </p>
 *
 * <h3>Remarks</h3>
 * <ul>
 *   <li>Note that this class is not synchronized.</li>
 *   <li>
 *     This comparator determines the order of instance identifiers by the
 *     target type. So two instance identifiers may differ even if this
 *     comparator returns zero.
 *   </li>
 * </ul>
 */
public final class IdentifierTargetComparator
    implements Comparator<InstanceIdentifier<?>>, Serializable {
    /**
     * Version number for serialization.
     */
    private static final long  serialVersionUID = 1L;

    /**
     * A map that keeps order of target types.
     */
    private final Map<Class<?>, Integer>  targetOrder = new HashMap<>();

    /**
     * Associate the order with the target type.
     *
     * @param type   A {@link Class} instance which indicates the target type.
     * @param order  The order to be associated with the given target type.
     * @return  This instance.
     */
    public IdentifierTargetComparator setOrder(Class<?> type, int order) {
        targetOrder.put(type, Integer.valueOf(order));
        return this;
    }

    /**
     * Return the order associated with the given identifier.
     *
     * @param type   A {@link Class} instance which indicates the target type.
     * @return  An {@link Integer} instance which represents the order
     *          associated with the given target type.
     *          {@code null} is returned if this instance does not contain
     *          the given target type.
     */
    public Integer getOrder(Class<?> type) {
        return targetOrder.get(type);
    }

    /**
     * Return the order associated with the given identifier.
     *
     * @param path  An {@link InstanceIdentifier} instance.
     * @return  An integer which represents the order associated with the
     *          given target type.
     * @throws NullPointerException
     *     {@code path} is {@code null}.
     */
    private int getOrder(InstanceIdentifier<?> path) {
        Integer value = getOrder(path.getTargetType());
        return (value == null) ? Integer.MAX_VALUE : value.intValue();
    }

    // Comparator

    /**
     * Compare the given {@link InstanceIdentifier} instances using its
     * target type.
     *
     * <p>
     *   This method compares order values associated with target types
     *   by {@link #setOrder(Class, int)}.
     * </p>
     *
     * @param o1  The first object to be compared.
     * @param o2  The second object to be compared.
     * @return  A negative integer, zero, or a positive integer as {@code o1}
     *          is less than, equal to, or greater than {@code o2}.
     * @throws NullPointerException
     *     {@code o1} or {@code o2} is {@code null}.
     */
    @Override
    public int compare(InstanceIdentifier<?> o1, InstanceIdentifier<?> o2) {
        int order1 = getOrder(o1);
        int order2 = getOrder(o2);
        return Integer.compare(order1, order2);
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
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        IdentifierTargetComparator comp = (IdentifierTargetComparator)o;
        return targetOrder.equals(comp.targetOrder);
    }

    // Object

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return targetOrder.hashCode() + (getClass().hashCode() * HASH_PRIME);
    }
}
