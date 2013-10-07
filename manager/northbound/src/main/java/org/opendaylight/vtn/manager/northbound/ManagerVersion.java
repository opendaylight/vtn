/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.BundleVersion;

/**
 * {@code ManagerVersion} class describes version information of the
 * VTN Manager.
 */
@XmlRootElement(name = "version")
@XmlAccessorType(XmlAccessType.NONE)
public class ManagerVersion {
    /**
     * API version of the VTN Manager.
     */
    @XmlAttribute(name = "api")
    private int  apiVersion;

    /**
     * Version of the OSGi bundle which implements the VTN Manager.
     */
    @XmlElement(name = "bundle")
    private BundleVersion  bundleVersion;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private ManagerVersion() {
    }

    /**
     * Construct a new manager version information.
     *
     * @param api     The API version number.
     * @param bundle  The version information of the OSGi bundle.
     */
    public ManagerVersion(int api, BundleVersion bundle) {
        apiVersion = api;
        bundleVersion = bundle;
    }

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
    public int getApiVersion() {
        return apiVersion;
    }

    /**
     * Return the version information of the OSGi bundle which implements
     * the VTN Manager.
     *
     * @return  A {@link BundleVersion} object which represents the version
     *          of the OSGi bundle which implements the VTN Manager.
     *          {@code null} is returned if the VTN Manager is loaded by
     *          a OSGi bundle class loader.
     */
    public BundleVersion getBundleVersion() {
        return bundleVersion;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!(o instanceof ManagerVersion)) {
            return false;
        }

        ManagerVersion mv = (ManagerVersion)o;
        if (apiVersion != mv.apiVersion) {
            return false;
        }

        if (bundleVersion == null) {
            return (mv.bundleVersion == null);
        }

        return bundleVersion.equals(mv.bundleVersion);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = apiVersion;
        if (bundleVersion != null) {
            h ^= bundleVersion.hashCode();
        }

        return h;
    }
}
