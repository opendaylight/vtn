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
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.controller.sal.core.Node;

/**
 * {@code VlanMapConfig} class describes configuration for the VLAN mapping.
 */
@XmlRootElement(name = "vlanmapconf")
@XmlAccessorType(XmlAccessType.NONE)
public class VlanMapConfig implements Serializable {
    private final static long serialVersionUID = 602803188863871510L;

    /**
     * Network element associated with the VLAN mapping.
     */
    @XmlElement
    private Node  node;

    /**
     * VLAN ID.
     */
    @XmlAttribute
    private short  vlan;

    /**
     * Private constructor used for JAXB mapping.
     */
    private VlanMapConfig() {
    }

    /**
     * Construct a new VLAN mapping configuration.
     *
     * @param node  Network element associated with the VLAN mapping.
     * @param vlan  VLAN ID to be mapped. Zero means that untagged Ethernet
     *              frames should be mapped.
     */
    public VlanMapConfig(Node node, short vlan) {
        this.node = node;
        this.vlan = vlan;
    }

    /**
     * Return a node associated with the VLAN mapping.
     *
     * @return  A node associated with the VLAN mapping.
     */
    public Node getNode() {
        return node;
    }

    /**
     * Return the VLAN ID to be mapped.
     *
     * @return  VLAN ID. Zero is returned if untagged Ethernet frame is
     *          configured.
     */
    public short getVlan() {
        return vlan;
    }

    /**
     * Determine whether the specified VLAN mapping configuration overlaps
     * the VLAN mapping represented by this object.
     *
     * @param vlconf  VLAN mapping configuration.
     * @return  {@code true} is returned only if the specified VLAN mapping
     *          overlaps this object.
     * @throws NullPointerException  {@code vlconf} is {@code null}.
     */
    public boolean isOverlapped(VlanMapConfig vlconf) {
        if (vlconf.vlan != vlan) {
            return false;
        }

        Node anotherNode = vlconf.node;
        return (node == null || anotherNode == null ||
                node.equals(anotherNode));
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
        if (!(o instanceof VlanMapConfig)) {
            return false;
        }

        VlanMapConfig vlconf = (VlanMapConfig)o;
        if (vlan != vlconf.vlan) {
            return false;
        }

        if (node == null) {
            return (vlconf.node == null);
        }

        return node.equals(vlconf.node);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = vlan + 271;

        if (node != null) {
            h += node.hashCode();
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
        StringBuilder builder = new StringBuilder("VlanMapConfig[");
        if (node != null) {
            builder.append("node=").append(node.toString()).append(",");
        }

        builder.append("vlan=").append((int)vlan).append(']');

        return builder.toString();
    }
}
