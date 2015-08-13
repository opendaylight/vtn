/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;

/**
 * {@code VBridgePath} class describes the position of the
 * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>} in the VTN.
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
public class VBridgePath extends VNodePath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 6995311908378312865L;

    /**
     * A string which represents that the node type is vBridge.
     */
    private static final String  NODETYPE_VBRIDGE = "vBridge";

    /**
     * Construct a new object which represents the position of the
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>} inside
     * the VTN.
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
        super(tenantName, bridgeName);
    }

    /**
     * Construct a new object which represents the position of the
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>} inside
     * the VTN.
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
        super(tenantPath, bridgeName);
    }

    /**
     * Return the name of the
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @return  The name of the vBridge.
     */
    public String getBridgeName() {
        return getTenantNodeName();
    }

    // VTenantPath

    /**
     * {@inheritDoc}
     *
     * @return  {@code "vBridge"} is always returned.
     * @since   Helium
     */
    @Override
    public String getNodeType() {
        return NODETYPE_VBRIDGE;
    }

    /**
     * {@inheritDoc}
     *
     * @since  Lithium
     */
    @Override
    public VirtualNodePath toVirtualNodePath() {
        return new VirtualNodePathBuilder().
            setTenantName(getTenantName()).
            setBridgeName(getTenantNodeName()).
            build();
    }

    // VNodePath

    /**
     * Convert this instance into a {@link VNodeLocation} instance.
     *
     * @return  A {@link VNodeLocation} instance.
     */
    @Override
    public VNodeLocation toVNodeLocation() {
        return new VNodeLocation(this);
    }
}
