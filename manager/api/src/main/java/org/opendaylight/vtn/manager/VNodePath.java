/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import java.util.List;

/**
 * {@code VNodePath} class describes the position of the
 * virtual node inside the
 * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
 *
 * <p>
 *   This class inherits {@link VTenantPath} and also stores the position
 *   information of the {@linkplain <a href="package-summary.html#VTN">VTN</a>}
 *   to which the virtual node belongs.
 * </p>
 *
 * @see  <a href="package-summary.html#vBridge">vBridge</a>
 * @since  Helium
 */
public abstract class VNodePath extends VTenantPath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 2181493384255939593L;

    /**
     * The name of the virtual node inside the VTN.
     */
    private final String  tenantNodeName;

    /**
     * Construct a new object which represents the position of the virtual
     * node inside the {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * <p>
     *   Exception will not occur even if incorrect name, such as {@code null},
     *   is specified to {@code tenantName}, but there will be error if you
     *   specify such instance in API of {@link IVTNManager} service.
     * </p>
     *
     * @param tenantName  The name of the VTN.
     * @param name        The name of the virtual node inside the VTN.
     */
    VNodePath(String tenantName, String name) {
        super(tenantName);
        tenantNodeName = name;
    }

    /**
     * Construct a new object which represents the position of the
     * virtual node inside the VTN.
     *
     * <p>
     *   This constructor specifies the VTN to which the virtual node belongs
     *   by using {@link VTenantPath}.
     * </p>
     *
     * @param tenantPath  A {@code VTenantPath} object that specifies the
     *                    position of the VTN.
     *                    All the values in the specified object are copied to
     *                    a new object.
     * @param name        The name of the virtual node.
     * @throws NullPointerException  {@code tenantPath} is {@code null}.
     */
    VNodePath(VTenantPath tenantPath, String name) {
        this(tenantPath.getTenantName(), name);
    }

    /**
     * Return the name of the virtual node inside the VTN.
     *
     * @return  The name of the virtual node inside the VTN.
     */
    public String getTenantNodeName() {
        return tenantNodeName;
    }

    /**
     * Convert this instance into a {@link VNodeLocation} instance.
     *
     * @return  A {@link VNodeLocation} instance.
     */
    public abstract VNodeLocation toVNodeLocation();

    // VTenantPath

    /**
     * {@inheritDoc}
     */
    @Override
    protected StringBuilder toStringBuilder() {
        StringBuilder builder = super.toStringBuilder();
        String name = tenantNodeName;
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
     *              An instance of {@code VBridgePath} must be specified.
     * @return   {@code true} if all path components in {@code path} are
     *           identical to components in this object.
     *           Otherwise {@code false}.
     */
    @Override
    protected boolean equalsPath(VTenantPath path) {
        if (!super.equalsPath(path)) {
            return false;
        }

        VNodePath npath = (VNodePath)path;
        if (tenantNodeName == null) {
            return (npath.tenantNodeName == null);
        }

        return tenantNodeName.equals(npath.tenantNodeName);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected int getHash() {
        int h = super.getHash();
        if (tenantNodeName != null) {
            h = h * HASH_PRIME + tenantNodeName.hashCode();
        }

        return h;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected List<String> getComponents() {
        List<String> components = super.getComponents();
        components.add(tenantNodeName);
        return components;
    }
}
