/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;

/**
 * {@code PortProperty} class describes properties of physical switch port
 * used by the VTN Manager.
 */
public class PortProperty implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -3079521309453101331L;

    /**
     * Name of the port.
     */
    private final String  name;

    /**
     * Determine whether this switch port is enabled or not.
     */
    private final boolean  enabled;

    /**
     * Construct a new switch port property.
     *
     * @param name      The name of the switch port.
     * @param enabled   {@code true} if the port is enabled.
     */
    public PortProperty(String name, boolean enabled) {
        this.name = name;
        this.enabled = enabled;
    }

    /**
     * Return the name of the switch port.
     *
     * @return  The name of the switch port.
     */
    public String getName() {
        return name;
    }

    /**
     * Determine whether the switch port is enabled or not.
     *
     * @return  {@code true} is returned if the switch port is enabled.
     *          Otherwise {@code false} is returned.
     */
    public boolean isEnabled() {
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
        if (!(o instanceof PortProperty)) {
            return false;
        }

        PortProperty pp = (PortProperty)o;
        if (name == null) {
            if (pp.name != null) {
                return false;
            }
        } else if (!name.equals(pp.name)) {
            return false;
        }

        return (enabled == pp.enabled);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = (enabled) ? 1 : 0;
        if (name != null) {
            h ^= name.hashCode();
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
        StringBuilder builder = new StringBuilder("PortProperty[");
        if (name != null) {
            builder.append("name=").append(name).append(',');
        }
        if (enabled) {
            builder.append("enabled");
        } else {
            builder.append("disabled");
        }

        return builder.append(']').toString();
    }
}
