/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

/**
 * {@code BundleVersion} class describes version information of the
 * VTN Manager OSGi bundle.
 */
@XmlRootElement(name = "bundleVersion")
@XmlAccessorType(XmlAccessType.NONE)
public class BundleVersion implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -768002871954697666L;

    /**
     * The number of bits to shift major version number for hash code.
     */
    private static final int  HASHCODE_SHIFT_MAJOR = 24;

    /**
     * The number of bits to shift minor version number for hash code.
     */
    private static final int  HASHCODE_SHIFT_MINOR = 16;

    /**
     * The major component of the bundle version identifier.
     */
    @XmlAttribute
    private int  major;

    /**
     * The minor component of the bundle version identifier.
     */
    @XmlAttribute
    private int  minor;

    /**
     * The micro component of the bundle version identifier.
     */
    @XmlAttribute
    private int  micro;

    /**
     * The qualifier component of the bundle version identifier.
     */
    @XmlAttribute
    private String  qualifier;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private BundleVersion() {
    }

    /**
     * Construct a new bundle version object.
     *
     * @param major      The major component of the bundle version.
     * @param minor      The minor component of the bundle version.
     * @param micro      The micro component of the bundle version.
     * @param qualifier  The qualifier component of the bundle version.
     *                   {@code null} can be specified if no qualifier is
     *                   defined.
     */
    public BundleVersion(int major, int minor, int micro, String qualifier) {
        this.major = major;
        this.minor = minor;
        this.micro = micro;
        this.qualifier = (qualifier != null && qualifier.isEmpty())
            ? null : qualifier;
    }

    /**
     * Return the major component of the bundle version.
     *
     * @return  The major component of the bundle version.
     */
    public int getMajor() {
        return major;
    }

    /**
     * Return the minor component of the bundle version.
     *
     * @return  The minor component of the bundle version.
     */
    public int getMinor() {
        return minor;
    }

    /**
     * Return the micro component of the bundle version.
     *
     * @return  The micro component of the bundle version.
     */
    public int getMicro() {
        return micro;
    }

    /**
     * Return the qualifier component of the bundle version.
     *
     * @return  The qualifier component of the bundle version.
     *          {@code null} is returned if no qualifier is defined.
     */
    public String getQualifier() {
        return qualifier;
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
        if (!(o instanceof BundleVersion)) {
            return false;
        }

        BundleVersion bv = (BundleVersion)o;
        if (major != bv.major || minor != bv.minor || micro != bv.micro) {
            return false;
        }

        if (qualifier == null) {
            return (bv.qualifier == null);
        }

        return qualifier.equals(bv.qualifier);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = (major << HASHCODE_SHIFT_MAJOR) |
            (minor << HASHCODE_SHIFT_MINOR) | micro;
        if (qualifier != null) {
            h ^= qualifier.hashCode();
        }

        return h;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder();
        char dot = '.';
        builder.append(major).append(dot).append(minor).append(dot).
            append(micro);
        if (qualifier != null) {
            builder.append(dot).append(qualifier);
        }

        return builder.toString();
    }
}
