/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.Objects;
import java.io.Serializable;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * {@code PortLocation} class describes the location of the physical
 * switch port.
 *
 * <p>
 *   This class provides JAXB mapping for a pair of {@link NodeConnector}
 *   instance and port name.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:00:00:01"
 * &nbsp;&nbsp;},
 * &nbsp;&nbsp;"port": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;"id": "1",
 * &nbsp;&nbsp;&nbsp;&nbsp;"name": "s1-eth1"
 * &nbsp;&nbsp;}
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "portlocation")
@XmlAccessorType(XmlAccessType.NONE)
public class PortLocation implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 5312439742053136468L;

    /**
     * Node information corresponding to the physical switch.
     *
     * <ul>
     *   <li>This element is mandatory.</li>
     * </ul>
     */
    @XmlElement(required = true)
    private Node  node;

   /**
     * Condition for identifying the port of the switch specified by
     * <strong>node</strong> element.
     *
     * <ul>
     *   <li>
     *     If omitted, this instance represents all ports within the physical
     *     switch specified by <strong>node</strong> element.
     *   </li>
     * </ul>
     */
    @XmlElement
    private SwitchPort  port;

    /**
     * Dummy constructor used for JAXB mapping and sub classes.
     */
    PortLocation() {
    }

    /**
     * Construct a new {@code PortLocation} instance which describes the
     * location of the physical switch port.
     *
     * @param node  {@link Node} object corresponding to the physical switch
     *              to be mapped by the port mapping.
     * @param port  A {@link SwitchPort} object that specifies the port within
     *              {@code node}. If {@code null} is specified, a new instance
     *              will represent all ports within the physical switch
     *              specified by {@code node}.
     */
    public PortLocation(Node node, SwitchPort port) {
        this.node = node;
        this.port = port;
    }

    /**
     * Construct a new {@code PortLocation} instance which represents the
     * location of the given {@link NodeConnector} instance.
     *
     * @param nc    A {@link NodeConnector} instance corresponding to
     *              the physical switch port.
     * @param name  The name of the port specified by {@code nc}.
     *              {@code null} means the port name is unavailable.
     * @throws NullPointerException
     *    {@code nc} is {@code null}.
     */
    public PortLocation(NodeConnector nc, String name) {
        this.node = nc.getNode();
        this.port = new SwitchPort(nc, name);
    }

    /**
     * Return a {@link Node} instance corresponding to the physical switch.
     *
     * @return  A {@link Node} instance corresponding to the physical switch.
     */
    public final Node getNode() {
        return node;
    }

    /**
     * Return a {@link SwitchPort} instance which identifies the port within
     * the physical switch.
     *
     * @return  A {@link SwitchPort} instance which identifies the physical
     *          switch port.
     */
    public final SwitchPort getPort() {
        return port;
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
        if (!(o instanceof PortLocation)) {
            return false;
        }

        PortLocation ploc = (PortLocation)o;
        return (Objects.equals(node, ploc.node) &&
                Objects.equals(port, ploc.port));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(node, port);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("PortLocation[");
        String sep = "";
        if (node != null) {
            builder.append("node=").append(node.toString());
            sep = ",";
        }
        if (port != null) {
            builder.append(sep).append("port=").append(port.toString());
        }
        builder.append(']');

        return builder.toString();
    }
}
