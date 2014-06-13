/*
 * Copyright (c) 2013-2014 NEC Corporation
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

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.controller.sal.core.Node;

/**
 * {@code PortMapConfig} class describes configuration information about the
 * port mapping.
 *
 * <p>
 *   This class is used to specify configuration information about the port
 *   mapping to the VTN Manager during configuration of port mapping.
 * </p>
 * <p>
 *   Actual physical port of switch that gets mapped to a vBridge interface is
 *   decided as follows.
 * </p>
 * <ul>
 *   <li>
 *     If the attribute <strong>{@link SwitchPort#getName() name}</strong> is
 *     configured in the element <strong>{@link #getPort() port}</strong>,
 *     then out of the physical ports in the switch specified in the element
 *     <strong>{@link #getNode() node}</strong>, the physical port that has
 *     the specified port name will get mapped.
 *   </li>
 *   <li>
 *     If the attributes <strong>{@link SwitchPort#getType() type}</strong>
 *     and <strong>{@link SwitchPort#getId() id}</strong> are configured in
 *     the element <strong>{@link #getPort() port}</strong>, then out of the
 *     physical ports in the switch specified in the element
 *     <strong>{@link #getNode() node}</strong>, the physical port
 *     corresponding to the specified
 *     {@link org.opendaylight.controller.sal.core.NodeConnector} will get
 *      mapped.
 *   </li>
 *   <li>
 *     If all the attributes are configured in the element
 *     <strong>{@link #getPort() port}</strong>, then out of the physical
 *     ports in the switch specified in the element
 *     <strong>{@link #getNode() node}</strong>, the physical port that meets
 *     all the condition will get mapped. That is, port mapping will be
 *     enabled only if the specified port name is configured for the specified
 *     {@link org.opendaylight.controller.sal.core.NodeConnector}.
 *   </li>
 * </ul>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"vlan": 10,
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
 * @see  <a href="package-summary.html#port-map">Port mapping</a>
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
     * Node information corresponding to the physical switch to be mapped
     * to the virtual interface.
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
     *   <li>This element is mandatory.</li>
     * </ul>
     */
    @XmlElement(name = "port", required = true)
    private SwitchPort  port;

    /**
     * VLAN ID to be mapped to the virtual interface.
     *
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>4095</strong>.
     *   </li>
     *   <li>
     *     <strong>0</strong> implies untagged ethernet frame.
     *   </li>
     *   <li>
     *     If omitted, it will be treated as <strong>0</strong> is specified.
     *   </li>
     * </ul>
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
     * Construct a new configuration information about the
     * {@linkplain <a href="package-summary.html#port-map">port mapping</a>}.
     *
     * @param node  {@link Node} object corresponding to the physical switch
     *              to be mapped by the port mapping.
     * @param port  A {@link SwitchPort} object that specifies the port within
     *              {@code node}.
     * @param vlan  VLAN ID to be mapped.
     *              <strong>0</strong> indicates that untagged ethernet frames
     *              should be mapped.
     */
    public PortMapConfig(Node node, SwitchPort port, short vlan) {
        this.node = node;
        this.port = port;
        this.vlan = vlan;
    }

    /**
     * Return the {@link Node} object corresponding to the physical switch
     * to be mapped by the
     * {@linkplain <a href="package-summary.html#port-map">port mapping</a>}.
     *
     * @return  The {@link Node} object corresponding to the physical switch
     *          to be mapped.
     */
    public Node getNode() {
        return node;
    }

    /**
     * Return a {@link SwitchPort} object which identifies the physical switch
     * port to be mapped by the
     * {@linkplain <a href="package-summary.html#port-map">port mapping</a>}.
     *
     * @return  A {@link SwitchPort} object which identifies the physical
     *          switch port to be mapped.
     */
    public SwitchPort getPort() {
        return port;
    }

    /**
     * Return the VLAN ID to be mapped.
     *
     * @return  VLAN ID to be mapped.
     *          <strong>0</strong> indicates that only untagged ethernet frames
     *          should be mapped.
     */
    public short getVlan() {
        return vlan;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code PortMapConfig} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>{@link Node} object corresponding to the physical switch.</li>
     *       <li>
     *         {@link SwitchPort} object which identifies the physical switch
     *         port.
     *       </li>
     *       <li>VLAN ID to be mapped.</li>
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
