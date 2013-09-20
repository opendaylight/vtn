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
 * {@code VInterfaceConfig} class describes configuration for an interface
 * attached to the virtual layer 2 bridge.
 */
@XmlRootElement(name = "interfaceconf")
@XmlAccessorType(XmlAccessType.NONE)
public class VInterfaceConfig implements Serializable {
    private static final long serialVersionUID = -6152398562520322469L;

    /**
     * An arbitrary description about the interface.
     */
    @XmlAttribute
    private String  description;

    /**
     * Determine whether the interface is enabled or not.
     */
    @XmlAttribute
    private Boolean  enabled;

    /**
     * Private constructor used for JAXB mapping.
     */
    private VInterfaceConfig() {
    }

    /**
     * Construct a new interface configuration.
     *
     * @param desc    Description about the interface.
     * @param enabled {@code Boolean.TRUE} means that the interface should be
     *                enabled.
     *                {@code Boolean.FALSE} means that the interface should be
     *                disabled.
     *                {@code null} is treated as default value or undefined
     *                value.
     */
    public VInterfaceConfig(String desc, Boolean enabled) {
        description = desc;
        this.enabled = enabled;
    }

    /**
     * Return description about the interface.
     *
     * @return  Description about the interface.
     *          {@code null} is returned if description is not set.
     */
    public String getDescription() {
        return description;
    }

    /**
     * Determine whether the interface is enabled or not.
     *
     * @return  {@code Boolean.TRUE} if the interface is enabled.
     *          {@code Boolean.FALSE} if the interface is disabled.
     *          {@code null} if interface state is not specified.
     */
    public Boolean getEnabled() {
        return enabled;
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
        if (!(o instanceof VInterfaceConfig)) {
            return false;
        }

        VInterfaceConfig iconf = (VInterfaceConfig)o;
        if (description == null) {
            if (iconf.description != null) {
                return false;
            }
        } else if (!description.equals(iconf.description)) {
            return false;
        }

        if (enabled == null) {
            return (iconf.enabled == null);
        }

        return enabled.equals(iconf.enabled);
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
        if (description != null) {
            builder.append("desc=").append(description);
        }

        if (enabled != null) {
            if (description != null) {
                builder.append(",");
            }

            if (enabled.booleanValue()) {
                builder.append("enabled");
            } else {
                builder.append("disabled");
            }
        }
        builder.append(']');

        return builder.toString();
    }
}
