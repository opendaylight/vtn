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
 * {@code VTenantConfig} class describes configuration information about the
 * VTN (virtual tenant network).
 *
 * <p>
 *   This class is used for specifying the VTN information to the VTN Manager
 *   during the creation or modification of the VTN.
 * </p>
 * <p>
 *   If a value greater than 0 is configured in both
 *   <strong>idleTimeout</strong> and <strong>hardTimeout</strong> attributes,
 *   then the value specified in <strong>hardTimeout</strong> must be greater
 *   than the value in <strong>idleTimeout</strong>.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"description": "Description about VTN 1",
 * &nbsp;&nbsp;"idleTimeout": "300",
 * &nbsp;&nbsp;"hardTimeout": "0"
 * }</pre>
 *
 * @see  <a href="package-summary.html#VTN">VTN</a>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vtnconf")
@XmlAccessorType(XmlAccessType.NONE)
public class VTenantConfig implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -2906937162063076543L;

    /**
     * An arbitrary description of the VTN.
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
     * The number of seconds to be set to {@code idle_timeout} field in
     * flow entries.
     */
    private int  idleTimeout;

    /**
     * The number of seconds to be set to {@code hard_timeout} field in
     * flow entries.
     */
    private int  hardTimeout;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private VTenantConfig() {
        idleTimeout = -1;
        hardTimeout = -1;
    }

    /**
     * Construct a new configuration information about the
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * <p>
     *   If this constructor is used, then flow entry timeout value becomes
     *   unset.
     * </p>
     *
     * @param desc  An arbitrary description of the VTN.
     *              Specifying {@code null} will imply that description is
     *              not configured for the VTN.
     */
    public VTenantConfig(String desc) {
        this(desc, -1, -1);
    }

    /**
     * Construct a new configuration information about the
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * <p>
     *   The value set in {@code hard} must be bigger than {@code idle} if the
     *   value set in both {@code idle} and {@code hard} is bigger than 0.
     *   Exception will not occur even if incorrect value is specified in
     *   {@code idle} or {@code hard}, but there will be error if you specify
     *   such {@code VTenantConfig} object in API of {@link IVTNManager}
     *   service.
     * </p>
     *
     * @param desc  An arbitrary description of the VTN.
     *              Specifying {@code null} will imply that description is
     *              not configured for the VTN.
     * @param idle
     *   The number of seconds that you want to configure in
     *   {@code idle_timeout} of flow entry configured by the VTN.
     *   <p>
     *     The flow entries configured in switch by the VTN are removed
     *     if they are not referred for the specified seconds.
     *   </p>
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       <strong>0</strong> means infinite time.
     *     </li>
     *     <li>
     *       Negative value will be ignored and treated as if no value is set.
     *     </li>
     *   </ul>
     * @param hard
     *   The number of seconds that you want to configure in
     *   {@code hard_timeout} of flow entry configured by the VTN.
     *   <p>
     *      The flow entries configured in switch by the VTN are removed
     *      after the specified time elapses.
     *   </p>
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       <strong>0</strong> means infinite time.
     *     </li>
     *     <li>
     *       Negative value will be ignored and treated as if no value is set.
     *     </li>
     *   </ul>
     */
    public VTenantConfig(String desc, int idle, int hard) {
        description = desc;
        idleTimeout = (idle < 0) ? -1 : idle;
        hardTimeout = (hard < 0) ? -1 : hard;
    }

    /**
     * Return the description of the.
     * {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *
     * @return  The description of the VTN.
     *          {@code null} is returned if description is not set.
     */
    public String getDescription() {
        return description;
    }

    /**
     * Return {@code idle_timeout} field value for flow entries.
     *
     * @return  {@code idle_timeout} value to be set to flow entries.
     *          -1 is returned if this object does not keep the value.
     */
    public int getIdleTimeout() {
        return idleTimeout;
    }

    /**
     * Return {@code hard_timeout} field value for flow entries.
     *
     * @return  {@code hard_timeout} value to be set to flow entries.
     *          -1 is returned if this object does not keep the value.
     */
    public int getHardTimeout() {
        return hardTimeout;
    }

    /**
     * Return an {@link Integer} object which represents {@code idle_timeout}
     * value for flow entries.
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
     *   The number of seconds that you want to configure in
     *   {@code idle_timeout} of flow entry configured by the VTN.
     *   The flow entries configured in switch by the VTN are removed
     *   if they are not referred for the specified seconds.
     *
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       <strong>0</strong> means infinite time.
     *     </li>
     *     <li>
     *       If a negative value is specified, then the specified value is
     *       ignored and it will be treated as if it is omitted.
     *     </li>
     *   </ul>
     * @deprecated  Only for JAXB. Use {@link #getIdleTimeout()} instead.
     */
    @XmlAttribute(name = "idleTimeout")
    public Integer getIdleTimeoutValue() {
        return (idleTimeout >= 0) ? Integer.valueOf(idleTimeout) : null;
    }

    /**
     * Set {@code idle_timeout} value for flow entries.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param idle  An {@link Integer} object which represents
     *              {@code idle_timeout} value for flow entries.
     */
    @SuppressWarnings("unused")
    private void setIdleTimeoutValue(Integer idle) {
        if (idle == null || idle.intValue() < 0) {
            idleTimeout = -1;
        } else {
            idleTimeout = idle.intValue();
        }
    }

    /**
     * Return an {@link Integer} object which represents {@code hard_timeout}
     * value for flow entries.
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
     *   The number of seconds that you want to configure in
     *   {@code hard_timeout} of flow entry configured by the VTN.
     *   The flow entries configured in switch by the VTN are removed
     *   after the specified time elapses.
     *
     *   <ul>
     *     <li>
     *       The range of value that can be specified is from
     *       <strong>0</strong> to <strong>65535</strong>.
     *     </li>
     *     <li>
     *       <strong>0</strong> means infinite time.
     *     </li>
     *     <li>
     *       If a negative value is specified, then the specified value is
     *       ignored and it will be treated as if it is omitted.
     *     </li>
     *   </ul>
     * @deprecated  Only for JAXB. Use {@link #getHardTimeout()} instead.
     */
    @XmlAttribute(name = "hardTimeout")
    public Integer getHardTimeoutValue() {
        return (hardTimeout >= 0) ? Integer.valueOf(hardTimeout) : null;
    }

    /**
     * Set {@code hard_timeout} value for flow entries.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param hard  An {@link Integer} object which represents
     *              {@code hard_timeout} value for flow entries.
     */
    @SuppressWarnings("unused")
    private void setHardTimeoutValue(Integer hard) {
        if (hard == null || hard.intValue() < 0) {
            hardTimeout = -1;
        } else {
            hardTimeout = hard.intValue();
        }
    }

    /**
     * Append human readable strings which represents the contents of this
     * object to the specified {@link StringBuilder}.
     *
     * @param builder  A {@link StringBuilder} instance.
     * @param prefix   A string to be inserted before contents.
     *                 {@code null} must not be specified.
     * @return  A {@link StringBuilder} instance specified by {@code builder}.
     */
    StringBuilder appendContents(StringBuilder builder, String prefix) {
        String pfx = prefix;
        if (description != null) {
            builder.append(pfx).append("desc=").append(description);
            pfx = ",";
        }

        if (idleTimeout >= 0) {
            builder.append(pfx).append("idleTimeout=").append(idleTimeout);
            pfx = ",";
        }

        if (hardTimeout >= 0) {
            builder.append(pfx).append("hardTimeout=").append(hardTimeout);
        }

        return builder;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code VTenantConfig} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         The description of the
     *         {@linkplain <a href="package-summary.html#VTN">VTN</a>}.
     *       </li>
     *       <li>
     *         The number of seconds for {@code idle_timeout} of flow entry.
     *       </li>
     *       <li>
     *         The number of seconds for {@code hard_timeout} of flow entry.
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
        if (!(o instanceof VTenantConfig)) {
            return false;
        }

        VTenantConfig tconf = (VTenantConfig)o;
        if (idleTimeout != tconf.idleTimeout ||
            hardTimeout != tconf.hardTimeout) {
            return false;
        }
        if (description == null) {
            return (tconf.description == null);
        }

        return description.equals(tconf.description);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = idleTimeout ^ hardTimeout;
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
        StringBuilder builder = new StringBuilder("VTenantConfig[");
        appendContents(builder, "").append(']');
        return builder.toString();
    }
}
