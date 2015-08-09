/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;
import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * {@code PathCost} class describes the cost of using specific switch link
 * for packet transmission.
 *
 * <p>
 *   This class is used to define {@link PathPolicy path policy}.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"location": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:00:00:01"
 * &nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;"port": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"name": "s1-eth1"
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"cost": 1000
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "pathcost")
@XmlAccessorType(XmlAccessType.NONE)
public final class PathCost implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 413188833974363845L;

    /**
     * The location of the physical switch port linked to another physical
     * switch.
     *
     * <ul>
     *   <li>
     *     If <strong>port</strong> element is not configured in this element,
     *     the link cost specified by <strong>cost</strong> attribute will be
     *     applied to all ports within the physical switch specified by
     *     <strong>location</strong> element.
     *   </li>
     * </ul>
     */
    @XmlElement(required = true)
    private PortLocation  location;

    /**
     * The cost of using physical switch port specified by
     * <strong>location</strong> element.
     *
     * <p>
     *   The specified cost value is used when a packet is transmitted from the
     *   switch port specified by <strong>location</strong> element.
     * </p>
     *
     * <ul>
     *   <li>
     *     The cost value must be greater than <strong>zero</strong>
     *   </li>
     * </ul>
     */
    @XmlAttribute(required = true)
    private Long  cost;

    /**
     * Private constructor defined only for JAXB.
     */
    private PathCost() {
    }

    /**
     * Construct a new instance.
     *
     * @param ploc  The location of the physical switch port.
     * @param c     The cost of using the port for transmitting.
     */
    public PathCost(PortLocation ploc, long c) {
        location = ploc;
        cost = Long.valueOf(c);
    }

    /**
     * Return the location of the physical switch port.
     *
     * @return  The location of the physical switch port.
     *          {@code null} is returned if not conigured.
     */
    public PortLocation getLocation() {
        return location;
    }

    /**
     * Return the cost of using physical switch port for transmitting.
     *
     * @return  The cost value.
     *          {@code null} is returned if not conigured.
     */
    public Long getCost() {
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
        if (!(o instanceof PathCost)) {
            return false;
        }

        PathCost pcost = (PathCost)o;
        return (Objects.equals(location, pcost.location) &&
                Objects.equals(cost, pcost.cost));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(location, cost);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("PathCost[");
        String sep = "";
        if (location != null) {
            builder.append("location=").append(location.toString());
            sep = ",";
        }
        if (cost != null) {
            builder.append(sep).append("cost=").append(cost.toString());
        }
        builder.append(']');

        return builder.toString();
    }
}
