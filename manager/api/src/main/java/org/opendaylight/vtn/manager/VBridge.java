/*
 * Copyright (c) 2013-2014 NEC Corporation
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

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * {@code VBridge} class describes the vBridge (virtual L2 bridge) information.
 *
 * <p>
 *   {@code VBridge} class inherits {@link VBridgeConfig} class, and
 *   {@code VBridge} object contains the vBridge configuration information.
 *   The VTN Manager passes the vBridge information to other components by
 *   passing {@code VBridge} object.
 * </p>
 *
 * @see  <a href="package-summary.html#vBridge">vBridge</a>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vbridge")
@XmlAccessorType(XmlAccessType.NONE)
public class VBridge extends VBridgeConfig {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -4764377113298832645L;

    /**
     * The name of the vBridge.
     */
    @XmlAttribute(required = true)
    private String  name;

    /**
     * State of the vBridge.
     *
     * @see  <a href="package-summary.html#vBridge.status">vBridge status</a>
     */
    private VNodeState  state;

    /**
     * The number of path faults detected inside the vBridge.
     *
     * <p>
     *   This shows the number of paths between the switches, constituting
     *   a vBridge, that could not be configured by the VTN Manager because
     *   the paths were broken.
     * </p>
     * <ul>
     *   <li>
     *     <strong>0</strong> gets configured when no path fault has been
     *     detected.
     *   </li>
     *   <li>
     *     The number of path fault is counted for each direction of path.
     *     For example, if switch A and switch B are separated, then the path
     *     from switch A to switch B and path from switch B to switch A are
     *     treated as different and both are counted.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private int  faults;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private VBridge() {
        super(null);
        state = VNodeState.UNKNOWN;
    }

    /**
     * Construct a new object which represents the
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}
     * information.
     *
     * @param bridgeName  The name of the vBridge.
     * @param state
     *   A {@link VNodeState} object which represents the
     *   {@linkplain <a href="package-summary.html#vBridge.status">state of the vBridge</a>}.
     *   The state will be treated as {@linkplain VNodeState#UNKNOWN UNKNOWN}
     *   if {@code null} is specified.
     * @param faults      The number of path faults detected inside the
     *                    vBridge.
     * @param bconf       A {@link VBridgeConfig} object which contains
     *                    configuration information about the vBridge.
     *                    All values in {@code bconf} get copied to a new
     *                    object.
     * @throws NullPointerException
     *    {@code bconf} is {@code null}.
     */
    public VBridge(String bridgeName, VNodeState state, int faults,
                   VBridgeConfig bconf) {
        super(bconf.getDescription(), bconf.getAgeInterval());
        name = bridgeName;
        this.faults = faults;

        if (state == null) {
            this.state = VNodeState.UNKNOWN;
        } else {
            this.state = state;
        }
    }

    /**
     * Return the name of the vBridge.
     *
     * @return  The name of the vBridge.
     */
    public String getName() {
        return name;
    }

    /**
     * Return the state of the
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * <ul>
     *   <li>
     *     {@link VNodeState#UP} is returned when the vBridge is in
     *     {@linkplain <a href="package-summary.html#vBridge.status.UP">UP</a>}
     *     state.
     *   </li>
     *   <li>
     *     {@link VNodeState#DOWN} is returned when the vBridge is in
     *     {@linkplain <a href="package-summary.html#vBridge.status.DOWN">DOWN</a>}
     *     state.
     *   </li>
     *   <li>
     *     {@link VNodeState#UNKNOWN} is returned when the vBridge is in
     *     {@linkplain <a href="package-summary.html#vBridge.status.UNKNOWN">UNKNOWN</a>}
     *     state.
     *   </li>
     * </ul>
     *
     * @return  A {@link VNodeState} object which represents the state of
     *          the vBridge.
     */
    public VNodeState getState() {
        return state;
    }

    /**
     * Return the number of path faults detected inside the
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *
     * <p>
     *   The value returned by this method shows the number of paths between
     *   the switches, constituting a vBridge, that could not be configured by
     *   the VTN Manager because the paths were broken.
     * </p>
     * <ul>
     *   <li>
     *     <strong>0</strong> is returned when no path fault has been detected.
     *   </li>
     *   <li>
     *     The number of path fault is counted for each direction of path.
     *     For example, if switch A and switch B are separated, then the path
     *     from switch A to switch B and path from switch B to switch A are
     *     treated as different and both are counted.
     *   </li>
     * </ul>
     *
     * @return  The number of path faults detected inside the vBridge.
     */
    public int getFaults() {
        return faults;
    }

    /**
     * Return a numerical representation of the vBridge state.
     *
     * <p>
     *   Note that below description of return value of this method is
     *   written for REST API document.
     * </p>
     *
     * @return
     *   The value showing the state of the vBridge gets configured.
     *
     *   <ul>
     *     <li>
     *       <strong>1</strong> gets configured when the vBridge is in
     *       {@link VNodeState#UP UP} state.
     *     </li>
     *     <li>
     *       <strong>0</strong> gets configured when the vBridge is in
     *       {@link VNodeState#DOWN DOWN} state.
     *     </li>
     *     <li>
     *       <strong>-1</strong> gets configured when the vBridge is in
     *       {@link VNodeState#UNKNOWN UNKNOWN} state.
     *     </li>
     *   </ul>
     * @deprecated  Only for JAXB. Use {@link #getState()} instead.
     */
    @XmlAttribute(name = "state")
    public int getStateValue() {
        return state.getValue();
    }

    /**
     * Set the state of the vBridge.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param st  A numerical representation of the vBridge state.
     */
    @SuppressWarnings("unused")
    private void setStateValue(int st) {
        this.state = VNodeState.valueOf(st);
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * <p>
     *   {@code true} is returned only if all the following conditions are met.
     * </p>
     * <ul>
     *   <li>
     *     {@code o} is a {@code vBridge} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         The name of the
     *         {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}.
     *       </li>
     *       <li>The description of the vBridge.</li>
     *       <li>
     *         The interval of
     *         {@linkplain <a href="package-summary.html#macTable.aging">MAC address table aging</a>}.
     *       </li>
     *       <li>The {@link #getState() state of the vBridge}.</li>
     *       <li>The number of path faults detected inside the vBridge.</li>
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
        if (!(o instanceof VBridge) || !super.equals(o)) {
            return false;
        }

        VBridge vbridge = (VBridge)o;
        if (state != vbridge.state) {
            return false;
        }
        if (faults != vbridge.faults) {
            return false;
        }

        if (name == null) {
            return (vbridge.name == null);
        }

        return name.equals(vbridge.name);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode() ^ state.toString().hashCode() + faults;
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
        StringBuilder builder = new StringBuilder("VBridge[");
        if (name != null) {
            builder.append("name=").append(name).append(',');
        }

        String desc = getDescription();
        if (desc != null) {
            builder.append("desc=").append(desc).append(',');
        }

        int age = getAgeInterval();
        if (age >= 0) {
            builder.append("ageInterval=").append(age).append(',');
        }

        builder.append("faults=").append(faults).
            append(",state=").append(state.toString()).append("]");

        return builder.toString();
    }
}
