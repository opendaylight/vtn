/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.List;

/**
 * {@code VBridgePath} class describes the position of the
 * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>} in the
 * container.
 *
 * <p>
 *   This class inherits {@link VTenantPath} and also stores the position
 *   information of the {@linkplain <a href="package-summary.html#VTN">VTN</a>}
 *   to which the vBridge belongs. An object of this class is used to identify
 *   the vBridge while executing any operations against it.
 * </p>
 *
 * @see  <a href="package-summary.html#vBridge">vBridge</a>
 */
public class VBridgePath extends VTenantPath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7691022408395210393L;

    /**
     * The name of the
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     */
    private final String  bridgeName;

    /**
     * Construct a new object which represents the position of the
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>} inside
     * the container.
     *
     * <p>
     *   Exception will not occur even if incorrect name, such as {@code null},
     *   is specified to {@code tenantName} or {@code bridgeName}, but there
     *   will be error if you specify such {@code VBridgePath} object in API
     *   of {@link IVTNManager} service.
     * </p>
     *
     * @param tenantName  The name of the VTN.
     * @param bridgeName  The name of the vBridge.
     */
    public VBridgePath(String tenantName, String bridgeName) {
        super(tenantName);
        this.bridgeName = bridgeName;
    }

    /**
     * Construct a new object which represents the position of the
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>} inside
     * the container.
     *
     * <p>
     *   This constructor specifies the VTN to which the vBridge belongs
     *   by using {@link VTenantPath}.
     * </p>
     *
     * <p>
     *   Exception will not occur even if incorrect name, such as {@code null},
     *   is specified to {@code bridgeName}, but there will be error if you
     *   specify such {@code VBridgePath} object in API of {@link IVTNManager}
     *   service.
     * </p>
     *
     * @param tenantPath  A {@code VTenantPath} object that specifies the
     *                    position of the VTN.
     *                    All the values in the specified object are copied to
     *                    a new object.
     * @param bridgeName  The name of the vBridge.
     * @throws NullPointerException  {@code tenantPath} is {@code null}.
     */
    public VBridgePath(VTenantPath tenantPath, String bridgeName) {
        this(tenantPath.getTenantName(), bridgeName);
    }

    /**
     * Return the name of the
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @return  The name of the vBridge.
     */
    public String getBridgeName() {
        return bridgeName;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected StringBuilder toStringBuilder() {
        StringBuilder builder = super.toStringBuilder();
        String name = bridgeName;
        if (name == null) {
            name = "<null>";
        }
        builder.append('.').append(name);

        return builder;
    }

    /**
     * Determine whether all components in the given path equal to components
     * in this object or not.
     *
     * @param path  An object to be compared.
     * @return   {@code true} if all path components in {@code path} are
     *           identical to components in this object.
     *           Otherwise {@code false}.
     */
    protected final boolean equalsPath(VBridgePath path) {
        if (!equalsPath((VTenantPath)path)) {
            return false;
        }

        if (bridgeName == null) {
            return (path.bridgeName == null);
        }

        return bridgeName.equals(path.bridgeName);
    }

    /**
     * {@inheritDoc}
     */
    protected List<String> getComponents() {
        List<String> components = super.getComponents();
        components.add(bridgeName);
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
     *     {@code o} is a {@code VBridgePath} object.
     *     Note that this method returns {@code false} if {@code o} is an
     *     object of subclass of {@code VBridgePath}.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         The name of the
     *         {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *       </li>
     *       <li>
     *         The name of the
     *         {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *       </li>
     *     </ul>
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

        return equalsPath((VBridgePath)o);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode();
        if (bridgeName != null) {
            h ^= bridgeName.hashCode();
        }

        return h;
    }
}
