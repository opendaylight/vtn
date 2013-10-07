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
 * {@code IVTNGlobal} provides methods for application to access
 * container-independent resources of Virtual Tenant Network (VTN).
 */
public interface IVTNGlobal {
    /**
     * Return the API version of the VTN Manager.
     *
     * <p>
     *   The API version will be incremented when changes which breaks
     *   compatibility is made to the API of VTN Manager.
     * </p>
     *
     * @return  The API version of the VTN Manager.
     */
    int getApiVersion();

    /**
     * Return the version information of the OSGi bundle which implements
     * the VTN Manager.
     *
     * @return  A {@link BundleVersion} object which represents the version
     *          of the OSGi bundle which implements the VTN Manager.
     *          {@code null} is returned if the VTN Manager is loaded by
     *          a OSGi bundle class loader.
     */
    BundleVersion getBundleVersion();
}
