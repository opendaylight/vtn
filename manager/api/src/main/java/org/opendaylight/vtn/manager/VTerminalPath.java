/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;

/**
 * {@code VTerminalPath} class describes the position of the
 * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>} in the
 * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
 *
 * <p>
 *   This class inherits {@link VTenantPath} and also stores the position
 *   information of the VTN to which the vTerminal belongs.
 *   An object of this class is used to identify the vTerminal while
 *   executing any operations against it.
 * </p>
 *
 * @see  <a href="package-summary.html#vTerminal">vTerminal</a>
 * @since  Helium
 */
public class VTerminalPath extends VNodePath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -534834320940791737L;

    /**
     * A string which represents that the node type is vTerminal.
     */
    private static final String  NODETYPE_VTERMINAL = "vTerminal";

    /**
     * Construct a new object which represents the position of the
     * vTerminal inside the VTN.
     *
     * <p>
     *   Exception will not occur even if incorrect name, such as {@code null},
     *   is specified to {@code tenantName} or {@code termName}, but there
     *   will be error if you specify such {@code VTerminalPath} object in API
     *   of {@link IVTNManager} service.
     * </p>
     *
     * @param tenantName  The name of the VTN.
     * @param termName    The name of the vTerminal.
     */
    public VTerminalPath(String tenantName, String termName) {
        super(tenantName, termName);
    }

    /**
     * Construct a new object which represents the position of the vTerminal
     * inside the VTN.
     *
     * <p>
     *   This constructor specifies the VTN to which the vTerminal belongs
     *   by using {@link VTenantPath}.
     * </p>
     *
     * <p>
     *   Exception will not occur even if incorrect name, such as {@code null},
     *   is specified to {@code termName}, but there will be error if you
     *   specify such {@code VTerminalPath} object in API of
     *   {@link IVTNManager} service.
     * </p>
     *
     * @param tenantPath  A {@code VTenantPath} object that specifies the
     *                    position of the VTN.
     *                    All the values in the specified object are copied to
     *                    a new object.
     * @param termName    The name of the vTerminal.
     * @throws NullPointerException  {@code tenantPath} is {@code null}.
     */
    public VTerminalPath(VTenantPath tenantPath, String termName) {
        super(tenantPath, termName);
    }

    /**
     * Return the name of the vTerminal.
     *
     * @return  The name of the vTerminal.
     */
    public String getTerminalName() {
        return getTenantNodeName();
    }

    // VTenantPath

    /**
     * {@inheritDoc}
     *
     * @return  {@code "vTerminal"} is always returned.
     * @since   Helium
     */
    @Override
    public String getNodeType() {
        return NODETYPE_VTERMINAL;
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
            setTerminalName(getTenantNodeName()).
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
