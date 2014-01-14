/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;

/**
 * {@code VTenantPath} class describes the position of the
 * {@linkplain <a href="package-summary.html#VTN">VTN</a>} inside the
 * container.
 *
 * <p>
 *   An object of this class is used to identify the VTN while executing any
 *   operations against it.
 * </p>
 *
 * @see  <a href="package-summary.html#VTN">VTN</a>
 */
public class VTenantPath implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 5295435248672675596L;

    /**
     * The name of the {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     */
    private final String  tenantName;

    /**
     * Construct a new object which represents the position of the
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>} inside the
     * container.
     *
     * <p>
     *   Exception will not occur even if incorrect name, such as {@code null},
     *   is specified to {@code tenantName}, but there will be error if you
     *   specify such {@code VTenantPath} object in API of {@link IVTNManager}
     *   service.
     * </p>
     *
     * @param tenantName  The name of the VTN.
     */
    public VTenantPath(String tenantName) {
        this.tenantName = tenantName;
    }

    /**
     * Return the name of the
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @return  The name of the VTN.
     */
    public String getTenantName() {
        return tenantName;
    }

    /**
     * Return a {@link StringBuilder} object which contains a string
     * representation of this object.
     *
     * <p>
     *   A string contained in a returned {@link StringBuilder} object is
     *   identical to a string returned by {@link #toString()}.
     * </p>
     *
     * @return  A {@link StringBuilder} object which contains a string
     *          representation of this object.
     */
    protected StringBuilder toStringBuilder() {
        String name = (tenantName == null) ? "<null>" : tenantName;
        return new StringBuilder(name);
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code VTenantPath} object.
     *   </li>
     *   <li>
     *     The name of the
     *     {@linkplain <a href="package-summary.html#VTN">VTN</a>} in {@code o}
     *     is equal to the value in this object.
     *   </li>
     * </ul>
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!(o instanceof VTenantPath)) {
            return false;
        }

        VTenantPath path = (VTenantPath)o;
        if (tenantName == null) {
            return (path.tenantName == null);
        }

        return tenantName.equals(path.tenantName);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (tenantName != null) {
            h ^= tenantName.hashCode();
        }

        return h;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        return toStringBuilder().toString();
    }
}
