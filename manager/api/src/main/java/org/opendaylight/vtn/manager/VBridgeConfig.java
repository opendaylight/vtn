/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * {@code VBridgeConfig} class describes configuration information about the
 * vBridge (virtual L2 bridge).
 *
 * <p>
 *   This class is used for specifying the vBridge information to the
 *   VTN Manager during the creation or modification of the vBridge.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"description": "Description about vBridge 1",
 * &nbsp;&nbsp;"ageInterval": "600"
 * }</pre>
 *
 * @see  <a href="package-summary.html#vBridge">vBridge</a>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vbridgeconf")
@XmlAccessorType(XmlAccessType.NONE)
public class VBridgeConfig implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 9009552129571959205L;

    /**
     * An arbitrary description of the vBridge.
     *
     * <ul>
     *   <li>
     *     There are no restrictions on the permissible characters or length
     *     of the string.
     *   </li>
     *   <li>
     *     The description is not configured if omitted.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private String  description;

    /**
     * The number of seconds between
     * {@linkplain <a href="package-summary.html#macTable.aging">MAC address table aging</a>}.
     */
    private int  ageInterval;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private VBridgeConfig() {
        ageInterval = -1;
    }

    /**
     * Construct a new configuration information about the
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * <p>
     *   The interval of
     *   {@linkplain <a href="package-summary.html#macTable.aging">MAC address table aging</a>}
     *   becomes unset if this constructor is used.
     * </p>
     *
     * @param desc  An arbitrary description of the vBridge.
     *              Specifying {@code null} will imply that description is
     *              not configured for the vBridge.
     */
    public VBridgeConfig(String desc) {
        this(desc, -1);
    }

    /**
     * Construct a new
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}
     * configuration.
     *
     * <p>
     *   Exception will not occur even if incorrect value is specified in
     *   {@code age}, but there will be error if you specify such
     *   {@code VBridgeConfig} object in API of {@link IVTNManager} service.
     * </p>
     *
     * @param desc  An arbitrary description of the vBridge.
     *              Specifying {@code null} will imply that description is
     *              not configured for the vBridge.
     * @param age
     *   The interval of
     *   {@linkplain <a href="package-summary.html#macTable.aging">MAC address table aging</a>}
     *   in seconds.
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>10</strong> to <strong>1000000</strong>.
     *     </li>
     *     <li>
     *       Negative value will be ignored and treated as if no value is set.
     *     </li>
     *   </ul>
     */
    public VBridgeConfig(String desc, int age) {
        description = desc;
        this.ageInterval = (age < 0) ? -1 : age;
    }

    /**
     * Return the description of the
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * @return  The description of the vBridge.
     *          {@code null} is returned if description is not set.
     */
    public String getDescription() {
        return description;
    }

    /**
     * Return the interval in seconds between
     * {@linkplain <a href="package-summary.html#macTable.aging">MAC address table aging</a>}.
     *
     * @return  The number of seconds between MAC address table aging.
     *          -1 is returned if this object does not keep the value.
     */
    public int getAgeInterval() {
        return ageInterval;
    }

    /**
     * Return an {@link Integer} object which represents the number of seconds
     * between MAC address table aging.
     *
     * <p>
     *   {@code null} is returned if this object does not keep the value.
     * </p>
     * <p>
     *   Note that below description of return value of this method is
     *   written for REST API document.
     * </p>
     *
     * @return
     *   The interval, in seconds, of aging for MAC address table in the
     *   vBridge.
     *
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>10</strong> to <strong>1000000</strong>.
     *     </li>
     *     <li>
     *       If a negative value is specified, then the specified value is
     *       ignored and it will be treated as if it is omitted.
     *     </li>
     *   </ul>
     * @deprecated  Only for JAXB. Use {@link #getAgeInterval()} instead.
     */
    @XmlAttribute(name = "ageInterval")
    public Integer getAgeIntervalValue() {
        return (ageInterval >= 0) ? Integer.valueOf(ageInterval) : null;
    }

    /**
     * Set the number of seconds between MAC address table aging.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param age  An {@link Integer} object which represents the number of
     *             seconds between MAC address table aging.
     */
    @SuppressWarnings("unused")
    private void setAgeIntervalValue(Integer age) {
        if (age == null || age.intValue() < 0) {
            ageInterval = -1;
        } else {
            ageInterval = age.intValue();
        }
    }

    /**
     * Append human readable strings which represents the contents of this
     * object to the specified {@link StringBuilder}.
     *
     * @param builder  A {@link StringBuilder} instance.
     * @param prefix   A string to be inserted before contents.
     *                 {@code null} must not be specified.
     * @return  {@code true} if at least one character is appended to the
     *          specified {@link StringBuilder} instance.
     *          Otherwise {@code false}.
     */
    boolean appendContents(StringBuilder builder, String prefix) {
        int len = builder.length();
        String pfx = prefix;
        if (description != null) {
            builder.append(pfx).append("desc=").append(description);
            pfx = ",";
        }
        if (ageInterval >= 0) {
            builder.append(pfx).append("ageInterval=").append(ageInterval);
        }

        return (builder.length() != len);
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code VBridgeConfig} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         The description of the
     *         {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *       </li>
     *       <li>
     *         The interval of
     *         {@linkplain <a href="package-summary.html#macTable.aging">MAC address table aging</a>}.
     *       </li>
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
        if (!(o instanceof VBridgeConfig)) {
            return false;
        }

        VBridgeConfig bconf = (VBridgeConfig)o;
        if (ageInterval != bconf.ageInterval) {
            return false;
        }

        if (description == null) {
            return (bconf.description == null);
        }

        return description.equals(bconf.description);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = ageInterval;
        if (description != null) {
            h ^= description.hashCode();
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
        StringBuilder builder = new StringBuilder("VBridgeConfig[");
        appendContents(builder, "");
        builder.append(']');
        return builder.toString();
    }
}
