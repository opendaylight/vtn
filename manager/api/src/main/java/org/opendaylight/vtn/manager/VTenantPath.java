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
import java.util.ArrayList;
import java.util.List;

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
public class VTenantPath implements Serializable, Comparable<VTenantPath> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -1044749796338168412L;

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
     * Determine whether the virtual tenant specified by this instance contains
     * the virtual node specified by {@code path}.
     *
     * <p>
     *   Note that this method returns {@code true} if the specified path
     *   is identical to this instance.
     * </p>
     *
     * @param path  A {@code VTenantPath} to be tested.
     * @return  {@code true} is returned only if the virtual tenant specified
     *          by this instance contains the virtual node specified by
     *          {@code path}.
     * @since   Helium
     */
    public final boolean contains(VTenantPath path) {
        return (getClass().isInstance(path) && equalsPath(path));
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
     * Determine whether all components in the given path equal to components
     * in this object or not.
     *
     * @param path  An object to be compared.
     *              Specifying {@code null} results in undefined behavior.
     * @return   {@code true} if all path components in {@code path} are
     *           identical to components in this object.
     *           Otherwise {@code false}.
     */
    protected boolean equalsPath(VTenantPath path) {
        if (tenantName == null) {
            return (path.tenantName == null);
        }

        return tenantName.equals(path.tenantName);
    }

    /**
     * Return a string list which contains all path components configured in
     * this instance.
     *
     * @return  A string list which contains all path components.
     * @since   Helium
     */
    protected List<String> getComponents() {
        ArrayList<String> components = new ArrayList<String>();
        components.add(tenantName);
        return components;
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
     *     Note that this method returns {@code false} if {@code o} is an
     *     object of subclass of {@code VTenantPath}.
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
        if (o == null || !o.getClass().equals(getClass())) {
            return false;
        }

        return equalsPath((VTenantPath)o);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = getClass().hashCode();
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
    public final String toString() {
        return toStringBuilder().toString();
    }

    // Comparable

    /**
     * Compare two {@code VTenantPath} instances numerically.
     *
     * <p>
     *   This method can compare not only an instance of {@code VTenantPath}
     *   but also an instance of class which inherits {@code VTenantPath}.
     *   Comparison will be done as follows.
     * </p>
     * <ol>
     *   <li>
     *     Compare the number of path components.
     *   </li>
     *   <li>
     *     Compare the name of the {@link Class} for both objects in
     *     dictionary order.
     *   </li>
     *   <li>
     *     Compare path components in dictionary order.
     *   </li>
     * </ol>
     *
     * @param  path  A {@code VTenantPath} instance to be compared.
     * @return {@code 0} is returned if this instance is equal to the
     *         specified instance.
     *         A value less than {@code 0} is returned if this instance is
     *         less than the specified instance.
     *         A value greater than {@code 0} is returned if this instance is
     *         greater than the specified instance.
     * @since  Helium
     */
    @Override
    public final int compareTo(VTenantPath path) {
        List<String> components = getComponents();
        List<String> otherComps = path.getComponents();
        int size = components.size();
        int otherSize = otherComps.size();
        int ret = size - otherSize;
        if (ret != 0) {
            return ret;
        }

        Class<?> cls = getClass();
        Class<?> otherCls = path.getClass();
        if (!cls.equals(otherCls)) {
            String name = cls.getName();
            String otherName = otherCls.getName();
            ret = name.compareTo(otherName);
            if (ret != 0) {
                return ret;
            }
        }

        for (int i = 0; i < size; i++) {
            String comp = components.get(i);
            String other = otherComps.get(i);
            if (comp == null) {
                if (other != null) {
                    return -1;
                }
            } else if (other == null) {
                return 1;
            } else {
                ret = comp.compareTo(other);
                if (ret != 0) {
                    return ret;
                }
            }
        }

        return ret;
    }
}
