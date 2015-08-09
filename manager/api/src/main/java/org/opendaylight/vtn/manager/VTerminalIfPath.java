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

import org.opendaylight.vtn.manager.flow.filter.RedirectFilter;

import org.opendaylight.controller.sal.core.UpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;

/**
 * {@code VTerminalIfPath} class describes the position of the
 * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
 * in the {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
 *
 * <p>
 *   This class inherits {@link VTerminalPath} and also stores the position
 *   information of the {@linkplain <a href="package-summary.html#VTN">VTN</a>}
 *   and the vTerminal to which the virtual interface belongs.
 *   An object of this class is used to identify the vTerminal interface while
 *   executing any operations against it.
 * </p>
 *
 * @see  <a href="package-summary.html#vTerminal">vTerminal</a>
 * @see  <a href="package-summary.html#vInterface">Virtual interface</a>
 * @since  Helium
 */
public class VTerminalIfPath extends VTerminalPath implements VInterfacePath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -8952168647370775777L;

    /**
     * A string which represents that the node type is vTerminal interface.
     */
    private static final String  NODETYPE_VTERM_IF = "vTerminal-IF";

    /**
     * The name of the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     */
    private final String  ifName;

    /**
     * Construct a new object which represents the position of the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * inside {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
     *
     * <p>
     *   Exception will not occur even if incorrect name, such as {@code null},
     *   is specified to {@code tenantName}, {@code termName}, or
     *   {@code ifName}, but there will be error if you specify such
     *   {@code VTerminalIfPath} object in API of {@link IVTNManager} service.
     * </p>
     *
     * @param tenantName  The name of the VTN.
     * @param termName    The name of the vTerminal.
     * @param ifName      The name of the virtual interface.
     */
    public VTerminalIfPath(String tenantName, String termName, String ifName) {
        super(tenantName, termName);
        this.ifName = ifName;
    }

    /**
     * Construct a new object which represents the position of the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * inside {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
     *
     * <p>
     *   This constructor specifies the vTerminal to which the virtual
     *   interface belongs by using {@link VTerminalPath}.
     * </p>
     *
     * <p>
     *   Exception will not occur even if incorrect name, such as {@code null},
     *   is specified to {@code ifName}, but there will be error if you
     *   specify such {@code VTerminalIfPath} object in API of
     *   {@link IVTNManager} service.
     * </p>
     *
     * @param termPath  A {@link VTerminalPath} object that specifies the
     *                  position of the vterminal.
     *                  All the values in the specified object are copied to
     *                  a new object.
     * @param ifName    The name of the virtual interface.
     * @throws NullPointerException  {@code termPath} is {@code null}.
     */
    public VTerminalIfPath(VTerminalPath termPath, String ifName) {
        this(termPath.getTenantName(), termPath.getTerminalName(), ifName);
    }

    // VTenantPath

    /**
     * {@inheritDoc}
     *
     * @return  {@code "vTerminal-IF"} is always returned.
     * @since   Helium
     */
    @Override
    public String getNodeType() {
        return NODETYPE_VTERM_IF;
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
            setInterfaceName(ifName).
            build();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected StringBuilder toStringBuilder() {
        StringBuilder builder = super.toStringBuilder();
        String name = ifName;
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
     *              An instance of {@code VTerminalIfPath} must be specified.
     * @return   {@code true} if all path components in {@code path} are
     *           identical to components in this object.
     *           Otherwise {@code false}.
     */
    @Override
    protected boolean equalsPath(VTenantPath path) {
        if (!super.equalsPath(path)) {
            return false;
        }

        VTerminalIfPath ipath = (VTerminalIfPath)path;
        if (ifName == null) {
            return (ipath.ifName == null);
        }

        return ifName.equals(ipath.ifName);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected int getHash() {
        int h = super.getHash();
        if (ifName != null) {
            h = h * HASH_PRIME + ifName.hashCode();
        }

        return h;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    protected List<String> getComponents() {
        List<String> components = super.getComponents();
        components.add(ifName);
        return components;
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

    // VInterfacePath

    /**
     * Return the name of the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     *
     * @return  The name of the virtual interface.
     */
    @Override
    public String getInterfaceName() {
        return ifName;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public VTerminalIfPath replaceTenantName(String tenantName) {
        return (VTerminalIfPath)super.replaceTenantName(tenantName);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public RedirectFilter getRedirectFilter(boolean out) {
        return new RedirectFilter(this, out);
    }

    /**
     * Call method that listens the change of the virtual interface.
     *
     * @param listener  An {@link IVTNManagerAware} instance.
     * @param viface    A {@link VInterface} instance which represents the
     *                  virtual interface information.
     * @param type      An {@link UpdateType} instance which indicates the
     *                  type of modification.
     * @throws NullPointerException
     *    {@code null} is passed to {@code listener}.
     * @see IVTNManagerAware#vInterfaceChanged(VTerminalIfPath, VInterface, UpdateType)
     * @since Helium
     */
    @Override
    public void vInterfaceChanged(IVTNManagerAware listener, VInterface viface,
                                  UpdateType type) {
        listener.vInterfaceChanged(this, viface, type);
    }

    /**
     * Call method that listens the change of port mapping configured the
     * virtual interface specified by this instance.
     *
     * @param listener  An {@link IVTNManagerAware} instance.
     * @param pmap      A {@link PortMap} instance which represents the
     *                  port mapping information.
     * @param type      An {@link UpdateType} instance which indicates the
     *                  type of modification.
     * @throws NullPointerException
     *    {@code null} is passed to {@code listener}.
     * @see IVTNManagerAware#portMapChanged(VTerminalIfPath, PortMap, UpdateType)
     * @since Helium
     */
    @Override
    public void portMapChanged(IVTNManagerAware listener, PortMap pmap,
                               UpdateType type) {
        listener.portMapChanged(this, pmap, type);
    }
}
