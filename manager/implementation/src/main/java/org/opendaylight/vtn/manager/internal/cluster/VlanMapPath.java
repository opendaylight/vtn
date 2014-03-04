/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VBridgePath;

/**
 * {@code VlanMapPath} class describes fully-qualified name of the VLAN mapping
 * applied to the virtual L2 bridge.
 *
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class VlanMapPath extends VBridgePath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -5858869883027125836L;

    /**
     * Identifier of the VLAN mapping.
     */
    private final String  mapId;

    /**
     * Construct a path to the VLAN mapping.
     *
     * @param bridgePath  Path to the virtual bridge.
     * @param mapId       The identifier of the VLAN mapping.
     *                    Specifying {@code null} results in undefined
     *                    behavior.
     * @throws NullPointerException  {@code bridgePath} is {@code null}.
     */
    public VlanMapPath(VBridgePath bridgePath, String mapId) {
        super(bridgePath.getTenantName(), bridgePath.getBridgeName());
        this.mapId = mapId;
    }

    /**
     * Return the identifier of the VLAN mapping.
     *
     * @return  Identifier of the VLAN mapping.
     */
    public String getMapId() {
        return mapId;
    }

    /**
     * Return a {@link StringBuilder} object which contains a string
     * representation of this object.
     *
     * @return  A {@link StringBuilder} object.
     */
    protected StringBuilder toStringBuilder() {
        StringBuilder builder = super.toStringBuilder();
        builder.append('.').append(mapId);

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
    protected final boolean equalsPath(VlanMapPath path) {
        if (!equalsPath((VBridgePath)path)) {
            return false;
        }

        return mapId.equals(path.mapId);
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
        if (o == null || !o.getClass().equals(VlanMapPath.class)) {
            return false;
        }

        return equalsPath((VlanMapPath)o);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() ^ mapId.hashCode();
    }
}
