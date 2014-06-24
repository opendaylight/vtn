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

import org.opendaylight.vtn.manager.VTenant;

/**
 * Comparator for {@link VTenant} instances, that compares the VTN name.
 */
public final class VTenantComparator implements Comparator<VTenant> {
    /**
     * Compare the given {@link VTenant} instances.
     *
     * @param t1  The first tenant to be compared.
     * @param t2  The second tenant to be compared.
     * @return    A negative integer, zero, or a positive integer as the
     *            first argument is less than, equal to, or greater than
     *            the second.
     */
    @Override
    public int compare(VTenant t1, VTenant t2) {
        String name1 = t1.getName();
        String name2 = t2.getName();

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
        return (o != null && VTenantComparator.class.equals(o.getClass()));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return VTenantComparator.class.hashCode();
    }
}
