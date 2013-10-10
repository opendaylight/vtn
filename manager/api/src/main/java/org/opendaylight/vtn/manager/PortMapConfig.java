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

import org.codehaus.jackson.map.annotate.JsonSerialize;

import org.opendaylight.controller.sal.core.Node;

/**
 * {@code PortMapConfig} class describes configuration for the port mapping.
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "portmapconf")
@XmlAccessorType(XmlAccessType.NONE)
public class PortMapConfig implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 1752903499492179337L;

    /**
     * Network element associated with the target switch.
     */
    @XmlElement
    private Node  node;

   /**
     * Condition to identify the target switch port.
     */
    @XmlElement(name = "port")
    private SwitchPort  port;

    /**
     * VLAN ID to be mapped to the virtual interface.
     */
    @XmlAttribute
    private short  vlan;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private PortMapConfig() {
    }

    /**
     * Construct a new port mapping information.
     *
     * @param node  Node associated with the target switch.
     * @param port  Condition to identify the target switch port.
     * @param vlan  VLAN ID to be mapped.
     *              Zero indicates that untagged Etnernet frames should be
     *              mapped.
     */
    public PortMapConfig(Node node, SwitchPort port, short vlan) {
        this.node = node;
        this.port = port;
        this.vlan = vlan;
    }

    /**
     * Return the node associated with the target switch.
     *
     * @return  Node associated with the target switch.
     */
    public Node getNode() {
        return node;
    }

    /**
     * Return a {@link SwitchPort} object which identifies the target switch
     * port.
     *
     * @return  A {@link SwitchPort} object.
     */
    public SwitchPort getPort() {
        return port;
    }

    /**
     * Return the VLAN ID to be mapped.
     *
     * @return  VLAN ID. Zero indicates that only untagged Ethernet frames
     *          should be mapped.
     */
    public short getVlan() {
        return vlan;
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
        if (!(o instanceof PortMapConfig)) {
            return false;
        }

        PortMapConfig pmconf = (PortMapConfig)o;
        if (vlan != pmconf.vlan) {
            return false;
        }

        if (node == null) {
            if (pmconf.node != null) {
                return false;
            }
        } else if (!node.equals(pmconf.node)) {
            return false;
        }

        if (port == null) {
            return (pmconf.port == null);
        }

        return port.equals(pmconf.port);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = vlan;
        if (node != null) {
            h ^= node.hashCode();
        }
        if (port != null) {
            h ^= port.hashCode();
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
        StringBuilder builder = new StringBuilder("PortMapConfig[");
        if (node != null) {
            builder.append("node=").append(node.toString()).append(',');
        }
        if (port != null) {
            builder.append("port=").append(port.toString()).append(',');
        }

        builder.append("vlan=").append((int)vlan).append(']');

        return builder.toString();
    }
}
