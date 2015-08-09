/*
 * Copyright (c) 2013, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.util.VTNIdentifiable;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeState;

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
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"name": "vbridge_1",
 * &nbsp;&nbsp;"description": "Description about vBridge 1",
 * &nbsp;&nbsp;"ageInterval": "600",
 * &nbsp;&nbsp;"faults": "0",
 * &nbsp;&nbsp;"state": "-1"
 * }</pre>
 *
 * @see  <a href="package-summary.html#vBridge">vBridge</a>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vbridge")
@XmlAccessorType(XmlAccessType.NONE)
public class VBridge extends VBridgeConfig implements VTNIdentifiable<String> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 3227947022547796551L;

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
    private VnodeState  state;

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
     * Convert the given integer value into a {@link VnodeState} instance.
     *
     * @param st  An integer value which represents the virtual node state.
     * @return   A {@link VnodeState} instance.
     */
    static VnodeState toVnodeState(int st) {
        VnodeState state = VnodeState.forValue(st);
        if (state == null) {
            state = VnodeState.UNKNOWN;
        }

        return state;
    }

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private VBridge() {
        super(null);
        state = VnodeState.UNKNOWN;
    }

    /**
     * Construct a new object which represents the
     * {@linkplain <a href="package-summary.html#vBridge">vBridge</a>}
     * information.
     *
     * @param bridgeName  The name of the vBridge.
     * @param state
     *   A {@link VnodeState} object which represents the
     *   {@linkplain <a href="package-summary.html#vBridge.status">state of the vBridge</a>}.
     *   The state will be treated as {@linkplain VnodeState#UNKNOWN UNKNOWN}
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
    public VBridge(String bridgeName, VnodeState state, int faults,
                   VBridgeConfig bconf) {
        super(bconf.getDescription(), bconf.getAgeInterval());
        name = bridgeName;
        this.faults = faults;

        if (state == null) {
            this.state = VnodeState.UNKNOWN;
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
     *     {@link VnodeState#UP} is returned when the vBridge is in
     *     {@linkplain <a href="package-summary.html#vBridge.status.UP">UP</a>}
     *     state.
     *   </li>
     *   <li>
     *     {@link VnodeState#DOWN} is returned when the vBridge is in
     *     {@linkplain <a href="package-summary.html#vBridge.status.DOWN">DOWN</a>}
     *     state.
     *   </li>
     *   <li>
     *     {@link VnodeState#UNKNOWN} is returned when the vBridge is in
     *     {@linkplain <a href="package-summary.html#vBridge.status.UNKNOWN">UNKNOWN</a>}
     *     state.
     *   </li>
     * </ul>
     *
     * @return  A {@link VnodeState} object which represents the state of
     *          the vBridge.
     */
    public VnodeState getState() {
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
     *       UP state.
     *     </li>
     *     <li>
     *       <strong>0</strong> gets configured when the vBridge is in
     *       DOWN state.
     *     </li>
     *     <li>
     *       <strong>-1</strong> gets configured when the vBridge is in
     *       UNKNOWN state.
     *     </li>
     *   </ul>
     * @deprecated  Only for JAXB. Use {@link #getState()} instead.
     */
    @XmlAttribute(name = "state")
    public int getStateValue() {
        return state.getIntValue();
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
        this.state = toVnodeState(st);
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
        return (state == vbridge.state && faults == vbridge.faults &&
                Objects.equals(name, vbridge.name));
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
        String prefix;
        if (name != null) {
            builder.append("name=").append(name);
            prefix = ",";
        } else {
            prefix = "";
        }

        if (appendContents(builder, prefix)) {
            prefix = ",";
        }
        builder.append(prefix).append("faults=").append(faults).
            append(",state=").append(state.toString()).append("]");

        return builder.toString();
    }

    // VTNIdentifiable

    /**
     * Return the identifier of this instance.
     *
     * <p>
     *   This class uses the name of the VBridge as the identifier.
     * </p>
     *
     * @return  The name of the VBridge.
     * @since   Lithium
     */
    @Override
    public String getIdentifier() {
        return name;
    }
}
