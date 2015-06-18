/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;

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
public class VTenantPath
    implements Serializable, Comparable<VTenantPath>, Cloneable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -5047395248346635881L;

    /**
     * A string which represents that the node type is VTN.
     */
    private static final String  NODETYPE_VTN = "VTN";

    /**
     * The name of the {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     */
    private String  tenantName;

    /**
     * Cache for the hash code of this instance.
     */
    private int  hash;

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
     * Return a human readable string which represents the type of the virtual
     * node corresponding to this instance.
     *
     * @return  {@code "VTN"} is always returned.
     * @since   Helium
     */
    public String getNodeType() {
        return NODETYPE_VTN;
    }

    /**
     * Create a shallow copy of this instance, and replace the tenant name
     * with the specified name.
     *
     * @param tenantName  The name of the VTN to be configured into a new
     *                    instance.
     * @return  A constructed instance.
     * @since   Helium
     */
    public VTenantPath replaceTenantName(String tenantName) {
        try {
            VTenantPath path = (VTenantPath)super.clone();
            path.tenantName = tenantName;

            // Need to clear cache for the hash code.
            path.hash = 0;
            return path;
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }

    /**
     * Convert this instance into a {@link VirtualNodePath} instance.
     *
     * @return  A {@link VirtualNodePath} instance.
     * @since   Lithium
     */
    public VirtualNodePath toVirtualNodePath() {
        return new VirtualNodePathBuilder().
            setTenantName(tenantName).build();
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
        StringBuilder builder = new StringBuilder(getNodeType());
        String name = (tenantName == null) ? "<null>" : tenantName;
        return builder.append(':').append(name);
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
     * Calculate the hash code of this object.
     *
     * @return  The hash code.
     */
    protected int getHash() {
        int h = getClass().getName().hashCode();
        if (tenantName != null) {
            h = h * HASH_PRIME + tenantName.hashCode();
        }

        return h;
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

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     The type of {@code o} exactly matches up to the type of this object.
     *   </li>
     *   <li>
     *     All path components in {@code o} are the same as this object.
     *   </li>
     * </ul>
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public final boolean equals(Object o) {
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
    public final int hashCode() {
        int h = hash;
        if (h == 0) {
            h = getHash();
            hash = h;
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
            assert !name.equals(otherName);
            return name.compareTo(otherName);
        }

        for (int i = 0; i < size; i++) {
            String comp = components.get(i);
            String other = otherComps.get(i);
            if (comp == null) {
                if (other != null) {
                    ret = -1;
                    break;
                }
            } else if (other == null) {
                ret = 1;
                break;
            } else {
                ret = comp.compareTo(other);
                if (ret != 0) {
                    break;
                }
            }
        }

        return ret;
    }

    // Cloneable

    /**
     * Create a shallow copy of this instance.
     *
     * @return  A shallow copy of this instance.
     */
    public VTenantPath clone() {
        try {
            return (VTenantPath)super.clone();
        } catch (CloneNotSupportedException e) {
            // This should never happen.
            throw new IllegalStateException("clone() failed", e);
        }
    }
}
