/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.opendaylight.vtn.manager.flow.filter.RedirectFilter;

import org.opendaylight.controller.sal.core.UpdateType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;

/**
 * {@code ErrorVNodePath} class is used to indicate that an invalid
 * virtual node path is specified.
 *
 * <p>
 *   This class is provided only for internal use.
 *   Java application must not use this class.
 * </p>
 *
 * @since  Helium
 */
public final class ErrorVNodePath extends VNodePath
    implements VInterfacePath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 8974666257098440684L;

    /**
     * A string which represents type type of this node path.
     */
    private static final String  NODETYPE_ERROR = "*** ERROR ***";

    /**
     * Construct a new instance.
     *
     * @param msg  An error message.
     */
    ErrorVNodePath(String msg) {
        super(msg, null);
    }

    /**
     * Return an error message configured in this instance.
     *
     * @return  An error message.
     */
    public String getError() {
        return getTenantName();
    }

    /**
     * Create an {@link IllegalStateException} that indicates unexpected method
     * is called.
     *
     * @return  An exception.
     */
    private IllegalStateException unexpected() {
        return new IllegalStateException("Should never be called.");
    }

    // VTenantPath

    /**
     * {@inheritDoc}
     */
    @Override
    public String getNodeType() {
        return NODETYPE_ERROR;
    }

    /**
     * This method should never be called.
     *
     * @return Never returns.
     * @throws IllegalStateException  Always thrown.
     * @since  Lithium
     */
    @Override
    public VirtualNodePath toVirtualNodePath() {
        throw unexpected();
    }

    // VNodePath

    /**
     * This method should never be called.
     *
     * @return  Never returns.
     * @throws IllegalStateException  Always thrown.
     */
    @Override
    public VNodeLocation toVNodeLocation() {
        throw unexpected();
    }

    // VInterfacePath

    /**
     * This method should never be called.
     *
     * @return  Never returns.
     * @throws IllegalStateException  Always thrown.
     */
    @Override
    public String getInterfaceName() {
        throw unexpected();
    }

    /**
     * This method should never be called.
     *
     * @param tenantName  Unused.
     * @return  Never returns.
     * @throws IllegalStateException  Always thrown.
     */
    @Override
    public ErrorVNodePath replaceTenantName(String tenantName) {
        throw unexpected();
    }

    /**
     * This method should never be called.
     *
     * @param out  Unused.
     * @return  Never returns.
     * @throws IllegalStateException  Always thrown.
     */
    @Override
    public RedirectFilter getRedirectFilter(boolean out) {
        throw unexpected();
    }

    /**
     * This method should never be called.
     *
     * @param listener  Unused.
     * @param viface    Unused.
     * @param type      Unused.
     * @throws IllegalStateException  Always thrown.
     */
    @Override
    public void vInterfaceChanged(IVTNManagerAware listener, VInterface viface,
                                  UpdateType type) {
        throw unexpected();
    }

    /**
     * This method should never be called.
     *
     * @param listener  Unused.
     * @param pmap      Unused.
     * @param type      Unused.
     * @throws IllegalStateException  Always thrown.
     */
    @Override
    public void portMapChanged(IVTNManagerAware listener, PortMap pmap,
                               UpdateType type) {
        throw unexpected();
    }
}
