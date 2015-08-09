/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

/**
 * {@code IVTNGlobal} is an interface that defines OSGi service for
 * providing container independent functionalities of the VTN Manager.
 *
 * <p>
 *   {@code IVTNGlobal} service is instantiated inside the OpenDaylight
 *   controller only once, and it is registered in the OSGi service registry
 *   as a global service.
 * </p>
 */
public interface IVTNGlobal {
    /**
     * Return the number which represents the API version of the VTN Manager.
     *
     * <p>
     *   The API version is a numerical value equal to or more than 1,
     *   and it gets incremented when the API of the VTN Manager is changed.
     * </p>
     *
     * @return  The API version of the VTN Manager.
     */
    int getApiVersion();

    /**
     * Return a {@link BundleVersion} object which represents the version
     * of the OSGi bundle which implements the VTN Manager.
     *
     * @return  A {@link BundleVersion} object which represents the version
     *          of the OSGi bundle which implements the VTN Manager.
     *          {@code null} is returned if the version of the OSGi bundle
     *          could not be retrieved.
     */
    BundleVersion getBundleVersion();
}
