/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

/**
 * {@code VBridgePath} class describes fully-qualified name of the virtual
 * layer 2 bridge in the virtual tenant.
 */
public class VBridgePath extends VTenantPath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 49188209943135523L;

    /**
     * The name of the bridge.
     */
    private final String  bridgeName;

    /**
     * Construct a path to the virtual L2 bridge.
     *
     * @param tenantName  The name of the tenant.
     * @param bridgeName  The name of the bridge.
     */
    public VBridgePath(String tenantName, String bridgeName) {
        super(tenantName);
        this.bridgeName = bridgeName;
    }

    /**
     * Construct a path to the virtual L2 bridge.
     *
     * @param tenantPath  Path to the virtual tenant.
     * @param bridgeName  The name of the bridge.
     * @throws NullPointerException  {@code tenantPath} is {@code null}.
     */
    public VBridgePath(VTenantPath tenantPath, String bridgeName) {
        this(tenantPath.getTenantName(), bridgeName);
    }

    /**
     * Return the name of the bridge.
     *
     * @return  The name of the bridge.
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
        if (!(o instanceof VBridgePath) || !super.equals(o)) {
            return false;
        }

        VBridgePath path = (VBridgePath)o;
        if (bridgeName == null) {
            return (path.bridgeName == null);
        }

        return bridgeName.equals(path.bridgeName);
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
