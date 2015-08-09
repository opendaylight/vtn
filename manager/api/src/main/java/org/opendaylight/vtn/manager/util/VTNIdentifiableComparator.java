/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.util;

import java.io.Serializable;
import java.util.Comparator;

/**
 * {@code VTNIdentifiableComparator} provides a comparison function for
 * {@link VTNIdentifiable} instances.
 *
 * <p>
 *   This class is used to sort {@link VTNIdentifiable} instances by natural
 *   ordering.
 * </p>
 *
 * @param <T>  The type of identifier.
 * @since  Lithium
 */
public final class VTNIdentifiableComparator<T extends Comparable<T>>
    implements Comparator<VTNIdentifiable<T>>, Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7249421452329586833L;

    /**
     * A class associated with the type of identifier.
     */
    private final Class<T>  identifierType;

    /**
     * Construct a new instance.
     *
     * @param type  A class associated with the type of identifier.
     */
    public VTNIdentifiableComparator(Class<T> type) {
        identifierType = type;
    }

    /**
     * Return the type of the identifier.
     *
     * @return  A class associated with the type of identifier.
     */
    public Class<T> getIdentifierType() {
        return identifierType;
    }

    /**
     * Compare the given two {@link VTNIdentifiable} instances.
     *
     * @param o1  The first object to be compared.
     * @param o2  The second object to be compared.
     * @return    A negative integer, zero, or a positive integer as the
     *            first argument is less than, equal to, or greater than
     *            the second.
     */
    @Override
    public int compare(VTNIdentifiable<T> o1, VTNIdentifiable<T> o2) {
        T id1 = o1.getIdentifier();
        T id2 = o2.getIdentifier();
        return id1.compareTo(id2);
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

        VTNIdentifiableComparator<?> comp =
            (VTNIdentifiableComparator<?>)o;
        return identifierType.equals(comp.identifierType);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return getClass().getName().hashCode() ^
            identifierType.getName().hashCode();
    }
}
