/*
 * Copyright (c) 2013-2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import java.io.Serializable;
import java.util.Map;

import org.opendaylight.vtn.manager.internal.MiscUtils;

import org.opendaylight.controller.sal.core.Bandwidth;
import org.opendaylight.controller.sal.core.Config;
import org.opendaylight.controller.sal.core.Name;
import org.opendaylight.controller.sal.core.Property;
import org.opendaylight.controller.sal.core.State;

/**
 * {@code PortProperty} class describes properties of physical switch port
 * used by the VTN Manager.
 */
public class PortProperty implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7279016516983307396L;

    /**
     * Base link speed used to calculate the link cost.
     */
    private static final long  BASE_BANDWIDTH = 10L * Bandwidth.BW1Tbps;

    /**
     * The minimum value of the link cost.
     */
    private static final long  LINK_COST_MIN = 1L;

    /**
     * Name of the port.
     */
    private final String  name;

    /**
     * Determine whether this switch port is enabled or not.
     */
    private final boolean  enabled;

    /**
     * The cost of the link for transmitting a packet from this port.
     */
    private final long  cost;

    /**
     * Construct a new switch port property.
     *
     * @param prop  A map which contains port properties.
     */
    public PortProperty(Map<String, Property> prop) {
        Bandwidth bw;
        if (prop == null) {
            // Property is unavailable.
            name = null;
            enabled = false;
            bw = null;
        } else {
            Name nm = (Name)prop.get(Name.NamePropName);
            name = (nm == null) ? null : nm.getValue();

            Config cf = (Config)prop.get(Config.ConfigPropName);
            State st = (State)prop.get(State.StatePropName);
            enabled =
                (cf != null && cf.getValue() == Config.ADMIN_UP &&
                 st != null && st.getValue() == State.EDGE_UP);
            bw = (Bandwidth)prop.get(Bandwidth.BandwidthPropName);
        }

        // Determine the cost of the link by link speed.
        // Use 1Gbps if the bandwidth is unavailable.
        long speed;
        if (bw == null) {
            speed = Bandwidth.BW1Gbps;
        } else {
            speed = bw.getValue();
            if (speed <= 0) {
                // This should never happen.
                speed = Bandwidth.BW1Gbps;
            }
        }

        long c = BASE_BANDWIDTH / speed;
        if (c < LINK_COST_MIN) {
            c = LINK_COST_MIN;
        }
        cost = c;
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
     * Return the cost of the link for transmitting a packet from this
     * switch port.
     *
     * @return  A long integer value which represents the cost.
     */
    public long getCost() {
        return cost;
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

        return (enabled == pp.enabled && cost == pp.cost);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = MiscUtils.hashCode(cost);
        if (enabled) {
            h *= 17;
        }
        if (name != null) {
            h += name.hashCode() * 31;
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
        builder.append("cost=").append(cost).append(',');
        if (enabled) {
            builder.append("enabled");
        } else {
            builder.append("disabled");
        }

        return builder.append(']').toString();
    }
}
