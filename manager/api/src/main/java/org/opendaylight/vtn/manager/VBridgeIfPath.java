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
 * {@code VBridgeIfPath} class describes fully-qualified name of the virtual
 * interface in the virtual L2 bridge.
 */
public class VBridgeIfPath extends VBridgePath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 9070687329308327585L;

    /**
     * The name of the interface.
     */
    private final String  ifName;

    /**
     * Construct a path to the virtual L2 bridge.
     *
     * @param tenantName  The name of the tenant.
     * @param bridgeName  The name of the bridge.
     * @param ifName      The name of the interface.
     */
    public VBridgeIfPath(String tenantName, String bridgeName, String ifName) {
        super(tenantName, bridgeName);
        this.ifName = ifName;
    }

    /**
     * Construct a path to the virtual L2 bridge.
     *
     * @param bridgePath  Path to the virtual bridge.
     * @param ifName      The name of the interface.
     * @throws NullPointerException  {@code bridgePath} is {@code null}.
     */
    public VBridgeIfPath(VBridgePath bridgePath, String ifName) {
        this(bridgePath.getTenantName(), bridgePath.getBridgeName(), ifName);
    }

    /**
     * Return the name of the interface.
     *
     * @return  The name of the interface.
     */
    public String getInterfaceName() {
        return ifName;
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
        if (!(o instanceof VBridgeIfPath) || !super.equals(o)) {
            return false;
        }

        VBridgeIfPath path = (VBridgeIfPath)o;
        if (ifName == null) {
            return (path.ifName == null);
        }

        return ifName.equals(path.ifName);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode();
        if (ifName != null) {
            h ^= ifName.hashCode();
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
        StringBuilder builder = new StringBuilder(super.toString());
        String name = ifName;
        if (name == null) {
            name = "<null>";
        }
        builder.append('.').append(name);

        return builder.toString();
    }
}
