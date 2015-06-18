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
 * {@code VInterfaceConfig} class describes configuration information about the
 * virtual interface.
 *
 * <p>
 *   This class is used for specifying the virtual interface information to the
 *   VTN Manager during the creation or modification of the virtual interface
 *   in vBridge.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"description": "Description about IF-1",
 * &nbsp;&nbsp;"enabled": true
 * }</pre>
 *
 * @see  <a href="package-summary.html#vInterface">Virtual interface</a>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "interfaceconf")
@XmlAccessorType(XmlAccessType.NONE)
public class VInterfaceConfig implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 4425846321318048040L;

    /**
     * An arbitrary description of the virtual interface.
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
     * A boolean value which determines whether the virtual interface is to
     * be enabled or not.
     *
     * <ul>
     *   <li>
     *     Specify {@code true} for enabling the virtual interface.
     *   </li>
     *   <li>
     *     Specify {@code false} for disabling the virtual interface.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private Boolean  enabled;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private VInterfaceConfig() {
    }

    /**
     * Construct a new configuration information about the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     *
     * @param desc    An arbitrary description of the virtual interface.
     *                Specifying {@code null} will imply that description is
     *                not configured for the virtual interface.
     * @param enabled
     *   A boolean value which determines whether the virtual interface is
     *   to be enabled or not.
     *   <ul>
     *     <li>
     *       Specify {@link Boolean#TRUE} for enabling and
     *       {@link Boolean#FALSE} for disabling the interface.
     *     </li>
     *     <li>
     *       Specifying {@code null} will imply that enable/disable
     *       configuration is not specified.
     *     </li>
     *   </ul>
     */
    public VInterfaceConfig(String desc, Boolean enabled) {
        description = desc;
        this.enabled = enabled;
    }

    /**
     * Return the description of the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     *
     * @return  The description of the virtual interface.
     *          {@code null} is returned if description is not set.
     */
    public String getDescription() {
        return description;
    }

    /**
     * Determine whether the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * is to be enabled or not.
     *
     * @return  {@link Boolean#TRUE} if the interface is configured as enable.
     *          {@link Boolean#FALSE} if the interface is configured as
     *          disable.
     *          {@code null} if enable/disable configuration is not specified.
     */
    public Boolean getEnabled() {
        return enabled;
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

        if (enabled != null) {
            builder.append(pfx);
            if (enabled.booleanValue()) {
                builder.append("enabled");
            } else {
                builder.append("disabled");
            }
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
     *     {@code o} is a {@code VInterfaceConfig} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         The description of the
     *         {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     *       </li>
     *       <li>Interface enable/disable configuration.</li>
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
        if (!(o instanceof VInterfaceConfig)) {
            return false;
        }

        VInterfaceConfig iconf = (VInterfaceConfig)o;
        return (Objects.equals(description, iconf.description) &&
                Objects.equals(enabled, iconf.enabled));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (description != null) {
            h ^= description.hashCode();
        }
        if (enabled != null) {
            h ^= enabled.hashCode();
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
        StringBuilder builder = new StringBuilder("VInterfaceConfig[");
        appendContents(builder, "");
        builder.append(']');
        return builder.toString();
    }
}
