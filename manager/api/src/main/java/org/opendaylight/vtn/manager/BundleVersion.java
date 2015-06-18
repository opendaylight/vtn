/*
 * Copyright (c) 2013-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;
import com.fasterxml.jackson.databind.annotation.JsonSerialize;
/**
 * {@code BundleVersion} class describes version information of the
 * VTN Manager OSGi bundle.
 *
 * <p>
 *   This class is used for passing OSGi bundle version of the VTN Manager to
 *   other components.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"api": "1",
 * &nbsp;&nbsp;"bundle": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"major": "0",
 * &nbsp;&nbsp;&nbsp;&nbsp;"minor": "1",
 * &nbsp;&nbsp;&nbsp;&nbsp;"micro": "0",
 * &nbsp;&nbsp;&nbsp;&nbsp;"qualifier": "SNAPSHOT"
 * &nbsp;&nbsp;}
 * }</pre>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
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
     * The major component of the OSGi bundle version identifier.
     */
    @XmlAttribute(required = true)
    private int  major;

    /**
     * The minor component of the OSGi bundle version identifier.
     */
    @XmlAttribute(required = true)
    private int  minor;

    /**
     * The micro component of the OSGi bundle version identifier.
     */
    @XmlAttribute(required = true)
    private int  micro;

    /**
     * The qualifier component of the OSGi bundle version identifier.
     *
     * <ul>
     *   <li>
     *     This attribute is omitted if the qualifier component is not set
     *     in the OSGi bundle version identifier.
     *   </li>
     * </ul>
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
     * Construct a new {@code BundleVersion} object which represents the
     * OSGi bundle version information.
     *
     * @param major      The major component of the OSGi bundle version.
     * @param minor      The minor component of the OSGi bundle version.
     * @param micro      The micro component of the OSGi bundle version.
     * @param qualifier  The qualifier component of the OSGi bundle version.
     *                   {@code null} can be specified if no qualifier is
     *                   defined in the OSGi bundle version.
     */
    public BundleVersion(int major, int minor, int micro, String qualifier) {
        this.major = major;
        this.minor = minor;
        this.micro = micro;
        this.qualifier = (qualifier != null && qualifier.isEmpty())
            ? null : qualifier;
    }

    /**
     * Return the major component of the OSGi bundle version.
     *
     * @return  The major component of the OSGi bundle version.
     */
    public int getMajor() {
        return major;
    }

    /**
     * Return the minor component of the OSGi bundle version.
     *
     * @return  The minor component of the OSGi bundle version.
     */
    public int getMinor() {
        return minor;
    }

    /**
     * Return the micro component of the OSGi bundle version.
     *
     * @return  The micro component of the OSGi bundle version.
     */
    public int getMicro() {
        return micro;
    }

    /**
     * Return the qualifier component of the OSGi bundle version.
     *
     * @return  The qualifier component of the OSGi bundle version.
     *          {@code null} is returned if no qualifier is defined.
     */
    public String getQualifier() {
        return qualifier;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code BundleVersion} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>The major component of the OSGi bundle version.</li>
     *       <li>The minor component of the OSGi bundle version.</li>
     *       <li>The micro component of the OSGi bundle version.</li>
     *       <li>The qualifier component of the OSGi bundle version.</li>
     *     </ul>
     *   </li>
     * </ul>
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
        return (major == bv.major && minor == bv.minor && micro == bv.micro &&
                Objects.equals(qualifier, bv.qualifier));
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
