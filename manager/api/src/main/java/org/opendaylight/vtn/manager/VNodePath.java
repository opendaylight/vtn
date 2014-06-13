/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

/**
 * {@code VNodePath} class describes the position of the
 * virtual node inside the
 * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
 *
 * <p>
 *   This class inherits {@link VTenantPath} and also stores the position
 *   information of the {@linkplain <a href="package-summary.html#VTN">VTN</a>}
 *   to which the virtual node belongs.
 * </p>
 *
 * @see  <a href="package-summary.html#vBridge">vBridge</a>
 * @since  Helium
 */
public abstract class VNodePath extends VTenantPath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 5947430862835452364L;

    /**
     * Construct a new object which represents the position of the virtual
     * node inside the {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * <p>
     *   Exception will not occur even if incorrect name, such as {@code null},
     *   is specified to {@code tenantName}, but there will be error if you
     *   specify such instance in API of {@link IVTNManager} service.
     * </p>
     *
     * @param tenantName  The name of the VTN.
     */
    protected VNodePath(String tenantName) {
        super(tenantName);
    }
}
