/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
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
 * {@code VlanMapConfig} class describes configuration information about the
 * VLAN mapping.
 *
 * <p>
 *   This class is used to specify configuration information about the VLAN
 *   mapping to the VTN Manager during configuration of VLAN mapping.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"vlan": "100",
 * &nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:00:00:03"
 * &nbsp;&nbsp;}
 * }</pre>
 *
 * @see  <a href="package-summary.html#VLAN-map">VLAN mapping</a>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vlanmapconf")
@XmlAccessorType(XmlAccessType.NONE)
public class VlanMapConfig implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 606475725018224880L;

    /**
     * {@link Node} information corresponding to the physical switch to be
     * mapped.
     *
     * <ul>
     *   <li>
     *     If this element is omitted, all switches managed by the OpenDaylight
     *     controller will be mapped.
     *   </li>
     * </ul>
     */
    @XmlElement
    private Node  node;

    /**
     * VLAN ID to be mapped.
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
    private VlanMapConfig() {
    }

    /**
     * Construct a new configuration information about the
     * {@linkplain <a href="package-summary.html#VLAN-map">VLAN mapping</a>}.
     *
     * @param node  {@link Node} object corresponding to the physical switch
     *              to be mapped by the VLAN mapping. Specify {@code null}
     *              if you want to map all switches instead of identifying
     *              particular switch.
     * @param vlan  VLAN ID to be mapped.
     *              <strong>0</strong> indicates that untagged ethernet frames
     *              should be mapped.
     */
    public VlanMapConfig(Node node, short vlan) {
        this.node = node;
        this.vlan = vlan;
    }

    /**
     * Return the {@link Node} object corresponding to the physical switch
     * to be mapped by the
     * {@linkplain <a href="package-summary.html#VLAN-map">VLAN mapping</a>}.
     *
     * @return  The {@link Node} object corresponding to the physical switch
     *          to be mapped. {@code null} is returned if switch is not
     *          specified.
     */
    public Node getNode() {
        return node;
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
     * Determine whether there is a duplicate of the VLAN mapped by
     * {@code VlanMapConfig} object.
     *
     * <p>
     *   This method compares the configuration information stored in
     *   {@code vlconf} and this object, and returns {@code true} only if all
     *   the following conditions are met.
     * </p>
     * <ul>
     *   <li>Same VLAN ID is configured in both objects.</li>
     *   <li>
     *     Same {@link Node} object is configured, or {@link Node} object
     *     is not configured in one of the two.
     *   </li>
     * </ul>
     *
     * @param vlconf  A {@code VlanMapConfig} object to be tested.
     * @return
     *   {@code true} is returned only if the
     *   {@linkplain <a href="package-summary.html#VLAN-map">VLAN mapping</a>}
     *   specified by {@code vlconf} overlaps the VLAN mapping specified by
     *    this object. Otherwise {@code false} is returned.
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
     * Append human readable strings which represents the contents of this
     * object to the specified {@link StringBuilder}.
     *
     * @param builder  A {@link StringBuilder} instance.
     * @return  A {@link StringBuilder} instance specified by {@code builder}.
     */
    StringBuilder appendContents(StringBuilder builder) {
        if (node != null) {
            builder.append("node=").append(node.toString()).append(",");
        }

        return builder.append("vlan=").append((int)vlan);
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code VlanMapConfig} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>{@link Node} object corresponding to the physical switch.</li>
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
        int h = vlan;
        if (node != null) {
            h ^= node.hashCode();
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
        appendContents(builder).append(']');
        return builder.toString();
    }
}
