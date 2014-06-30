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

import org.opendaylight.vtn.manager.internal.cluster.PathMapImpl;

/**
 * Comparator for {@link PathMapImpl} instances, that compares the index.
 */
public final class PathMapImplComparator implements Comparator<PathMapImpl> {
    /**
     * Compare the given {@link PathMapImpl} instances.
     *
     * @param p1  The first path map to be compared.
     *            A valid index must be configured in the specified instance.
     * @param p2  The second path map to be compared.
     *            A valid index must be configured in the specified instance.
     * @return    A negative integer, zero, or a positive integer as the
     *            first argument is less than, equal to, or greater than
     *            the second.
     */
    @Override
    public int compare(PathMapImpl p1, PathMapImpl p2) {
        int idx1 = p1.getIndex();
        int idx2 = p2.getIndex();

        return Integer.compare(idx1, idx2);
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        return (o != null && PathMapImplComparator.class.equals(o.getClass()));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return PathMapImplComparator.class.hashCode();
    }
}
