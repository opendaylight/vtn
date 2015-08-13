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

/**
 * {@code VInterfacePath} defines methods to be implemented by classes which
 * represents the position of the
 * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
 *
 * <p>
 *   This interface is provided only for internal use.
 *   Java application must not define new class that implements this
 *   interface.
 * </p>
 *
 * @see  <a href="package-summary.html#vInterface">Virtual interface</a>
 * @since  Helium
 */
public interface VInterfacePath {
    /**
     * Return the name of the
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @return  The name of the VTN.
     */
    String getTenantName();

    /**
     * Return the name of the virtual node that contains the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * specified by this instance.
     *
     * @return  The name of the virtual node that contains the virtual
     *          interface.
     */
    String getTenantNodeName();

    /**
     * Return the name of the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     *
     * @return  The name of the virtual interface.
     */
    String getInterfaceName();

    /**
     * Return a human readable string which represents the type of the virtual
     * node which contains the virtual interface.
     *
     * @return  A string which represents the type of the virtual node.
     */
    String getNodeType();

    /**
     * Convert this instance into a {@link VNodeLocation} instance.
     *
     * @return  A {@link VNodeLocation} instance.
     */
    VNodeLocation toVNodeLocation();

    /**
     * Create a shallow copy of this instance, and replace the tenant name
     * with the specified name.
     *
     * @param tenantName  The name of the VTN to be configured into a new
     *                    instance.
     * @return  A constructed instance.
     */
    VInterfacePath replaceTenantName(String tenantName);

    /**
     * Create a {@link RedirectFilter} instance which represents the
     * packet redirection to the virtual interface specified by this
     * instance.
     *
     * @param out {@code true} means the redirected packet should be treated
     *            as outgoing packet.
     *            {@code false} means the redirected packet should be treated
     *            as incoming packet.
     * @return  A {@link RedirectFilter} instance.
     */
    RedirectFilter getRedirectFilter(boolean out);

    /**
     * Call method that listens the change of the virtual interface.
     *
     * @param listener  An {@link IVTNManagerAware} instance.
     * @param viface    A {@link VInterface} instance which represents the
     *                  virtual interface information.
     * @param type      An {@link UpdateType} instance which indicates the
     *                  type of modification.
     */
    void vInterfaceChanged(IVTNManagerAware listener, VInterface viface,
                           UpdateType type);

    /**
     * Call method that listens the change of port mapping configured the
     * virtual interface specified by this instance.
     *
     * @param listener  An {@link IVTNManagerAware} instance.
     * @param pmap      A {@link PortMap} instance which represents the
     *                  port mapping information.
     * @param type      An {@link UpdateType} instance which indicates the
     *                  type of modification.
     */
    void portMapChanged(IVTNManagerAware listener, PortMap pmap,
                        UpdateType type);
}
