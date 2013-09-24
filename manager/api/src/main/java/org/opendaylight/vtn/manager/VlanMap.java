/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.controller.sal.core.Node;

/**
 * {@code VlanMap} class describes information about the VLAN mapping.
 */
@XmlRootElement(name = "vlanmap")
@XmlAccessorType(XmlAccessType.NONE)
public class VlanMap extends VlanMapConfig {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7225775555870161356L;

    /**
     * Unique identifier assigned to the VLAN mapping.
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
     * Construct a new VLAN mapping information.
     *
     * @param id    Unique identifier of the VLAN mapping.
     * @param node  Node associated with the VLAN mapping.
     * @param vlan  VLAN ID to be mapped. Zero means that untagged Ethernet
     *              frames should be mapped.
     */
    public VlanMap(String id, Node node, short vlan) {
        super(node, vlan);
        this.id = id;
    }

    /**
     * Return the identifier of the VLAN mapping.
     *
     * @return  The identifier of the VLAN mapping.
     */
    public String getId() {
        return id;
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

        Node node = getNode();
        if (node != null) {
            builder.append("node=").append(node).append(',');
        }

        builder.append("vlan=").append((int)getVlan()).append(']');

        return builder.toString();
    }
}
