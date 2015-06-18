/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
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
 * {@code VTerminal} class describes the vTerminal information.
 *
 * <p>
 *   {@code VTerminal} class inherits {@link VTerminalConfig} class, and
 *   {@code VTerminal} object contains the vTerminal configuration information.
 *   The VTN Manager passes the vTerminal information to other components by
 *   passing {@code VTerminal} object.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"name": "vterm_1",
 * &nbsp;&nbsp;"description": "Description about vTerminal 1",
 * &nbsp;&nbsp;"faults": "0",
 * &nbsp;&nbsp;"state": "-1"
 * }</pre>
 *
 * @see  <a href="package-summary.html#vTerminal">vTerminal</a>
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vterminal")
@XmlAccessorType(XmlAccessType.NONE)
public class VTerminal extends VTerminalConfig
    implements VTNIdentifiable<String> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -8426139969576246558L;

    /**
     * The name of the vTerminal.
     */
    @XmlAttribute(required = true)
    private String  name;

    /**
     * State of the vTerminal.
     *
     * @see  <a href="package-summary.html#vTerminal.status">vTerminal status</a>
     */
    private VnodeState  state;

    /**
     * The number of path faults in flows redirected to the vTerminal
     * interface.
     *
     * <p>
     *   This shows the number of broken paths redirected to the vTerminal
     *   interface by flow filter.
     * </p>
     * <ul>
     *   <li>
     *     <strong>0</strong> gets configured when no path fault has been
     *     detected.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private int  faults;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private VTerminal() {
        super(null);
        state = VnodeState.UNKNOWN;
    }

    /**
     * Construct a new object which represents the
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}
     * information.
     *
     * @param termName  The name of the vTerminal.
     * @param state
     *   A {@link VnodeState} object which represents the
     *   {@linkplain <a href="package-summary.html#vTerminal.status">state of the vTerminal</a>}.
     *   The state will be treated as {@linkplain VnodeState#UNKNOWN UNKNOWN}
     *   if {@code null} is specified.
     * @param faults    The number of path faults in flows redirected to the
     *                  vTerminal interface.
     * @param vtconf    A {@link VTerminalConfig} object which contains
     *                  configuration information about the vTerminal.
     *                  All values in {@code vtconf} get copied to a new object.
     * @throws NullPointerException
     *    {@code vtconf} is {@code null}.
     */
    public VTerminal(String termName, VnodeState state, int faults,
                     VTerminalConfig vtconf) {
        super(vtconf.getDescription());
        name = termName;
        this.faults = faults;

        if (state == null) {
            this.state = VnodeState.UNKNOWN;
        } else {
            this.state = state;
        }
    }

    /**
     * Return the name of the vTerminal.
     *
     * @return  The name of the vTerminal.
     */
    public String getName() {
        return name;
    }

    /**
     * Return the state of the
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
     *
     * <ul>
     *   <li>
     *     {@link VnodeState#UP} is returned when the vTerminal is in
     *     {@linkplain <a href="package-summary.html#vTerminal.status.UP">UP</a>}
     *     state.
     *   </li>
     *   <li>
     *     {@link VnodeState#DOWN} is returned when the vTerminal is in
     *     {@linkplain <a href="package-summary.html#vTerminal.status.DOWN">DOWN</a>}
     *     state.
     *   </li>
     *   <li>
     *     {@link VnodeState#UNKNOWN} is returned when the vTerminal is in
     *     {@linkplain <a href="package-summary.html#vTerminal.status.UNKNOWN">UNKNOWN</a>}
     *     state.
     *   </li>
     * </ul>
     *
     * @return  A {@link VnodeState} object which represents the state of
     *          the vTerminal.
     */
    public VnodeState getState() {
        return state;
    }

    /**
     * Return the number of path faults in flows redirected to the
     * {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}
     * interface.
     *
     * <p>
     *   The value returned by this method shows the number of broken paths
     *   redirected to the vTerminal interface by flow filter.
     * </p>
     * <ul>
     *   <li>
     *     <strong>0</strong> is returned when no path fault has been detected.
     *   </li>
     * </ul>
     *
     * @return  The number of path faults.
     */
    public int getFaults() {
        return faults;
    }

    /**
     * Return a numerical representation of the vTerminal state.
     *
     * <p>
     *   Note that below description of return value of this method is
     *   written for REST API document.
     * </p>
     *
     * @return
     *   The value showing the state of the vTerminal gets configured.
     *
     *   <ul>
     *     <li>
     *       <strong>1</strong> gets configured when the vTerminal is in
     *       UP state.
     *     </li>
     *     <li>
     *       <strong>0</strong> gets configured when the vTerminal is in
     *       DOWN state.
     *     </li>
     *     <li>
     *       <strong>-1</strong> gets configured when the vTerminal is in
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
     * Set the state of the vTerminal.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param st  A numerical representation of the vTerminal state.
     */
    @SuppressWarnings("unused")
    private void setStateValue(int st) {
        this.state = VBridge.toVnodeState(st);
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
     *     {@code o} is a {@code vTerminal} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         The name of the
     *         {@linkplain <a href="package-summary.html#vTerminal">vTerminal</a>}.
     *       </li>
     *       <li>The description of the vTerminal.</li>
     *       <li>The {@link #getState() state of the vTerminal}.</li>
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
        if (!(o instanceof VTerminal) || !super.equals(o)) {
            return false;
        }

        VTerminal vterm = (VTerminal)o;
        return (state == vterm.state && faults == vterm.faults &&
                Objects.equals(name, vterm.name));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode() + (state.toString().hashCode() * 11) +
            (faults * 13);
        if (name != null) {
            h += name.hashCode() * 23;
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
        StringBuilder builder = new StringBuilder("VTerminal[");
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

        return builder.append(prefix).append("faults=").append(faults).
            append(",state=").append(state.toString()).
            append("]").toString();
    }

    // VTNIdentifiable

    /**
     * Return the identifier of this instance.
     *
     * <p>
     *   This class uses the name of the VTerminal as the identifier.
     * </p>
     *
     * @return  The name of the VTerminal.
     * @since   Lithium
     */
    @Override
    public String getIdentifier() {
        return name;
    }
}
