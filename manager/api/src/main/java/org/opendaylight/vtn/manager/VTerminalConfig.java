/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
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
 * {@code VTerminalConfig} class describes configuration information about the
 * vTerminal in the VTN.
 *
 * <p>
 *   This class is used for specifying the vTerminal information to the
 *   VTN Manager during the creation or modification of the vTerminal.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"description": "Description about vTerminal 1",
 * }</pre>
 *
 * @see  <a href="package-summary.html#vTerminal">vTerminal</a>
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vterminalconf")
@XmlAccessorType(XmlAccessType.NONE)
public class VTerminalConfig implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7330164264616964987L;

    /**
     * An arbitrary description of the vTerminal.
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
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private VTerminalConfig() {
    }

    /**
     * Construct a new configuration information about the
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
     *
     * @param desc  An arbitrary description of the vTerminal.
     *              Specifying {@code null} will imply that description is
     *              not configured for the vTerminal.
     */
    public VTerminalConfig(String desc) {
        description = desc;
    }

    /**
     * Return the description of the
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
     *
     * @return  The description of the vTerminal.
     *          {@code null} is returned if description is not set.
     */
    public String getDescription() {
        return description;
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
        if (description != null) {
            builder.append(prefix).append("desc=").append(description);
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
     *     {@code o} is a {@code VTerminalConfig} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         The description of the
     *         {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
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
        if (!(o instanceof VTerminalConfig)) {
            return false;
        }

        VTerminalConfig vtconf = (VTerminalConfig)o;
        if (description == null) {
            return (vtconf.description == null);
        }

        return description.equals(vtconf.description);
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

        return h;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("VTerminalConfig[");
        appendContents(builder, "");
        builder.append(']');
        return builder.toString();
    }
}
