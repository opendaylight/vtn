/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import org.opendaylight.controller.sal.core.UpdateType;

/**
 * {@code VInterfacePath} defines methods to be implemented by classes which
 * represents the position of the
 * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
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
