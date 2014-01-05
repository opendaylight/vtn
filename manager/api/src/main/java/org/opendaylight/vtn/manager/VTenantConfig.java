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

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * {@code VTenantConfig} class describes configuration for the the virtual
 * tenant.
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vtnconf")
@XmlAccessorType(XmlAccessType.NONE)
public class VTenantConfig implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7446090329841381143L;

    /**
     * An arbitrary description about the tenant.
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
     * Construct a new tenant configuration.
     *
     * <p>
     *   Timeout value of flow entries are determine by the system.
     * </p>
     *
     * @param desc  Description about the tenant.
     */
    public VTenantConfig(String desc) {
        this(desc, -1, -1);
    }

    /**
     * Construct a new tenant configuration.
     *
     * @param desc  Description about the tenant.
     * @param idle  The number of seconds to be set to {@code idle_timeout}
     *              field in flow entries. Zero means an infinite timeout.
     *              The value is determined by the system if a negative value
     *              is passed.
     * @param hard  The number of seconds to be set to {@code hard_timeout}
     *              field in flow entries. Zero means an infinite timeout.
     *              The value is determined by the system if a negative value
     *              is passed.
     */
    public VTenantConfig(String desc, int idle, int hard) {
        description = desc;
        idleTimeout = (idle < 0) ? -1 : idle;
        hardTimeout = (hard < 0) ? -1 : hard;
    }

    /**
     * Return description about the tenant.
     *
     * @return  Description about the tenant.
     *          {@code null} is returned if description is not set.
     */
    public String getDescription() {
        return description;
    }

    /**
     * Return {@code idle_timeout} field value for flow entries.
     *
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
     * Return an {@code Integer} object which represents {@code idle_timeout}
     * value for flow entries.
     *
     * @return  An {@code Integer} object which represents {@code idle_timeout}
     *          value. {@code null} is returned if this object does not keep
     *          the value.
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
     * @param idle  An {@code Integer} object which represents
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
     * Return an {@code Integer} object which represents {@code hard_timeout}
     * value for flow entries.
     *
     * @return  An {@code Integer} object which represents {@code hard_timeout}
     *          value. {@code null} is returned if this object does not keep
     *          the value.
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
     * @param hard  An {@code Integer} object which represents
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
        char sep = 0;
        if (description != null) {
            builder.append("desc=").append(description);
            sep = ',';
        }

        if (idleTimeout >= 0) {
            if (sep != 0) {
                builder.append(sep);
            }
            builder.append("idleTimeout=").append(idleTimeout);
            sep = ',';
        }

        if (hardTimeout >= 0) {
            if (sep != 0) {
                builder.append(sep);
            }
            builder.append("hardTimeout=").append(hardTimeout);
        }
        builder.append(']');

        return builder.toString();
    }
}
