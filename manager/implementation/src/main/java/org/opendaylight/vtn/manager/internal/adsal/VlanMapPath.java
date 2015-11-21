/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.adsal;

import static org.opendaylight.vtn.manager.util.NumberUtils.HASH_PRIME;

import java.util.List;

import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.VBridgePath;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfoBuilder;

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
public class VlanMapPath extends VBridgeMapPath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -6594224353793282012L;

    /**
     * A string which represents that the node type is VLAN mapping.
     */
    private static final String  NODETYPE_VLANMAP = "VlanMap";

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
        super(bridgePath);
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

    // VBridgeMapPath

    /**
     * Return a {@link BridgeMapInfo} instance which represents the virtual
     * network mapping information.
     *
     * @return  A {@link BridgeMapInfo} instance which contains the VLAN
     *          mapping information.
     */
    @Override
    public BridgeMapInfo getBridgeMapInfo() {
        return new BridgeMapInfoBuilder().setVlanMapId(mapId).build();
    }

    // VTenantPath

    /**
     * {@inheritDoc}
     *
     * @return  {@code "VlanMap"} is always returned.
     */
    @Override
    public String getNodeType() {
        return NODETYPE_VLANMAP;
    }

    /**
     * Return a {@link StringBuilder} object which contains a string
     * representation of this object.
     *
     * @return  A {@link StringBuilder} object.
     */
    @Override
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
     *              An instance of {@code VlanMapPath} must be specified.
     * @return   {@code true} if all path components in {@code path} are
     *           identical to components in this object.
     *           Otherwise {@code false}.
     */
    @Override
    protected boolean equalsPath(VTenantPath path) {
        if (!super.equalsPath(path)) {
            return false;
        }

        VlanMapPath vpath = (VlanMapPath)path;
        return mapId.equals(vpath.mapId);
    }

    /**
     * Calculate the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    protected int getHash() {
        return (super.getHash() * HASH_PRIME) + mapId.hashCode();
    }

    /**
     * Return a string list which contains all path components configured in
     * this instance.
     *
     * @return  A string list which contains all path components.
     * @since   Helium
     */
    @Override
    protected List<String> getComponents() {
        List<String> components = super.getComponents();
        components.add(mapId);
        return components;
    }
}
