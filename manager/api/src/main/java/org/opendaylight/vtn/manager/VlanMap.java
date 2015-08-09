/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.util.VTNIdentifiable;

import org.opendaylight.controller.sal.core.Node;

/**
 * {@code VlanMap} class describes information about the VLAN mapping, which
 * maps a VLAN to a vBridge.
 *
 * <p>
 *   {@code VlanMap} class inherits {@link VlanMapConfig} class, and
 *   {@code VlanMap} object contains the configuration information about the
 *   VLAN mapping. The VTN Manager passes the VLAN mapping information to
 *   other components by passing {@code VlanMap} object.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"id": "OF-00:00:00:00:00:00:00:03.0",
 * &nbsp;&nbsp;"vlan": "0",
 * &nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:00:00:03"
 * &nbsp;&nbsp;}
 * }</pre>
 *
 * @see  <a href="package-summary.html#VLAN-map">VLAN mapping</a>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vlanmap")
@XmlAccessorType(XmlAccessType.NONE)
public class VlanMap extends VlanMapConfig implements VTNIdentifiable<String> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 6332981215061576792L;

    /**
     * The identifier assigned to the VLAN mapping, which is unique in the
     * vBridge.
     */
    @XmlAttribute(required = true)
    private String  id;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private VlanMap() {
        this(null, null, (short)0);
    }

    /**
     * Construct a new object which represents information about the
     * {@linkplain <a href="package-summary.html#VLAN-map">VLAN mapping</a>}.
     *
     * @param id    The identifier assigned to the the VLAN mapping.
     * @param node  A {@link Node} object specified in the VLAN mapping
     *              configuration.
     * @param vlan  The VLAN ID specified in the VLAN mapping configuration.
     */
    public VlanMap(String id, Node node, short vlan) {
        super(node, vlan);
        this.id = id;
    }

    /**
     * Return the identifier of the
     * {@linkplain <a href="package-summary.html#VLAN-map">VLAN mapping</a>}.
     *
     * @return  The identifier of the VLAN mapping.
     */
    public String getId() {
        return id;
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code VlanMap} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         {@link Node} object corresponding to the physical switch to be
     *         mapped.
     *       </li>
     *       <li>VLAN ID to be mapped.</li>
     *       <li>
     *         The identifier of the
     *         {@linkplain <a href="package-summary.html#VLAN-map">VLAN mapping</a>}.
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
        if (!(o instanceof VlanMap) || !super.equals(o)) {
            return false;
        }

        VlanMap vlanmap = (VlanMap)o;
        if (id == null) {
            return (vlanmap.id == null);
        }

        return id.equals(vlanmap.id);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode();
        if (id != null) {
            h ^= id.hashCode();
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
        StringBuilder builder = new StringBuilder("VlanMap[");
        if (id != null) {
            builder.append("id=").append(id).append(',');
        }

        appendContents(builder).append(']');
        return builder.toString();
    }

    // VTNIdentifiable

    /**
     * Return the identifier of this instance.
     *
     * <p>
     *   This method returns the identifier of the VLAN mapping.
     * </p>
     *
     * @return  The identifier of the VLAN mapping.
     * @since   Lithium
     */
    @Override
    public String getIdentifier() {
        return id;
    }
}
