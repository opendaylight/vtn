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
 * {@code VInterface} class describes the virtual interface information.
 *
 * <p>
 *   {@code VInterface} class inherits {@link VInterfaceConfig} class, and
 *   {@code VInterface} object contains the virtual interface configuration
 *   information. The VTN Manager passes the virtual interface information to
 *   other components by passing {@code VInterface} object.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"name": "if_1",
 * &nbsp;&nbsp;"state": "-1",
 * &nbsp;&nbsp;"entityState": "-1",
 * &nbsp;&nbsp;"description": "Description about IF-1",
 * &nbsp;&nbsp;"enabled": "true"
 * }</pre>
 *
 * @see  <a href="package-summary.html#vInterface">Virtual interface</a>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "interface")
@XmlAccessorType(XmlAccessType.NONE)
public class VInterface extends VInterfaceConfig
    implements VTNIdentifiable<String> {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 2939100529362804426L;

    /**
     * The name of the virtual interface.
     */
    @XmlAttribute(required = true)
    private String  name;

    /**
     * State of the virtual interface.
     *
     * @see  <a href="package-summary.html#vInterface.status">Virtual interface status</a>
     */
    private VnodeState  state;

    /**
     * State of the network element mapped to this virtual interface.
     */
    private VnodeState  entityState;

    /**
     * Private constructor used for JAXB mapping.
     */
    @SuppressWarnings("unused")
    private VInterface() {
        super(null, null);
        state = VnodeState.UNKNOWN;
        entityState = VnodeState.UNKNOWN;
    }

    /**
     * Construct a new object which represents the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}
     * information.
     *
     * @param ifName  The name of the virtual interface.
     * @param state
     *  A {@link VnodeState} object which represents the
     *  {@linkplain <a href="package-summary.html#vInterface.status">state of the virtual interface</a>}.
     *  The state will be treated as {@linkplain VnodeState#UNKNOWN UNKNOWN} if
     *  {@code null} is specified.
     * @param estate  A {@link VnodeState} object which represents the state
     *                of the network element mapped to the virtual interface.
     *                The state will be treated as
     *                {@linkplain VnodeState#UNKNOWN UNKNOWN} if {@code null}
     *                is specified.
     * @param iconf   A {@link VInterfaceConfig} object which contains
     *                configuration information about the virtual interface.
     *                All values in {@code iconf} get copied to a new object.
     * @throws NullPointerException
     *    {@code iconf} is {@code null}.
     */
    public VInterface(String ifName, VnodeState state, VnodeState estate,
                      VInterfaceConfig iconf) {
        super(iconf.getDescription(), iconf.getEnabled());
        name = ifName;

        if (state == null) {
            this.state = VnodeState.UNKNOWN;
        } else {
            this.state = state;
        }
        if (estate == null) {
            this.entityState = VnodeState.UNKNOWN;
        } else {
            this.entityState = estate;
        }
    }

    /**
     * Return the name of the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     *
     * @return  The name of the virtual interface.
     */
    public String getName() {
        return name;
    }

    /**
     * Return the state of the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     *
     * <ul>
     *   <li>
     *     {@link VnodeState#UP} is returned when the virtual interface is in
     *     {@linkplain <a href="package-summary.html#vInterface.status.UP">UP</a>}
     *      state.
     *   </li>
     *   <li>
     *     {@link VnodeState#DOWN} is returned when the virtual interface is in
     *     {@linkplain <a href="package-summary.html#vInterface.status.DOWN">DOWN</a>}
     *      state.
     *   </li>
     *   <li>
     *     {@link VnodeState#UNKNOWN} is returned when the virtual interface is
     *     in
     *     {@linkplain <a href="package-summary.html#vInterface.status.UNKNOWN">UNKNOWN</a>}
     *     state.
     *   </li>
     * </ul>
     *
     * @return  A {@link VnodeState} object which represents the state of
     *          the virtual interface.
     */
    public VnodeState getState() {
        return state;
    }

    /**
     * Return the state of the network element mapped to the
     * {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     *
     * <p>
     *   If the
     *   {@linkplain <a href="package-summary.html#port-map">port mapping</a>} is
     *   configured to the virtual interface specified by this object,
     *   this method returns a {@link VnodeState} object which represents the
     *   state of the physical switch port mapped to the virtual interface.
     * </p>
     *
     * <ul>
     *   <li>
     *     {@link VnodeState#UP} is returned if mapped network elements is
     *     operating.
     *   </li>
     *   <li>
     *     {@link VnodeState#DOWN} is returned if mapped network elements is
     *     not operating.
     *   </li>
     *   <li>
     *     {@link VnodeState#UNKNOWN} is returned if no network element is
     *     mapped to the virtual interface.
     *   </li>
     * </ul>
     *
     * <p>
     *   Note that the state of mapped network element is configured
     *   irrespective of whether the virtual interface is enabled or disabled.
     *   For example, even if the virtual interface is disabled, this method
     *   returns {@link VnodeState#UP} if mapped network element is operating.
     * </p>
     *
     * @return  The state of the network element mapped to this interface.
     */
    public VnodeState getEntityState() {
        return entityState;
    }

    /**
     * Return a numerical representation of the virtual interface state.
     *
     * <p>
     *   Note that below description of return value of this method is
     *   written for REST API document.
     * </p>
     *
     * @return
     *   The value showing the state of the virtual interface gets configured.
     *
     *   <ul>
     *     <li>
     *       <strong>1</strong> gets configured when the virtual interface is
     *       in UP state.
     *     </li>
     *     <li>
     *       <strong>0</strong> gets configured when the virtual interface is
     *       in DOWN state.
     *     </li>
     *     <li>
     *       <strong>-1</strong> gets configured when the virtual interface is
     *       in UNKNOWN state.
     *     </li>
     *   </ul>
     * @deprecated  Only for JAXB. Use {@link #getState()} instead.
     */
    @XmlAttribute(name = "state")
    public int getStateValue() {
        return state.getIntValue();
    }

    /**
     * Set the state of the virtual interface.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param st  A numerical representation of the virtual interface state.
     */
    @SuppressWarnings("unused")
    private void setStateValue(int st) {
        this.state = VBridge.toVnodeState(st);
    }

    /**
     * Return a numerical representation of the state of the network element
     * mapped to the virtual interface.
     *
     * <p>
     *   Note that below description of return value of this method is
     *   written for REST API document.
     * </p>
     *
     * @return
     *   The state of the network element mapped to the virtual interface gets
     *   configured.
     *   <p>
     *     If the port mapping is configured to the virtual interface specified
     *     by this object, then the state of the physical switch port mapped
     *     to the virtual interface gets configured.
     *   </p>
     *   <ul>
     *     <li>
     *       <strong>1</strong> gets configured if mapped network element is
     *       operating.
     *     </li>
     *     <li>
     *       <strong>0</strong> gets configured if mapped network element is
     *       not operating.
     *     </li>
     *     <li>
     *       <strong>-1</strong> gets configured if no network element is
     *       mapped to the virtual interface.
     *     </li>
     *   </ul>
     *   <p>
     *     Note that the state of mapped network element is configured
     *     irrespective of whether the virtual interface is enabled or
     *     disabled. For example, even if the virtual interface is disabled,
     *     <strong>1</strong> will be configured if mapped network element is
     *     operating.
     *   </p>
     * @deprecated  Only for JAXB. Use {@link #getEntityState()} instead.
     */
    @XmlAttribute(name = "entityState")
    public int getEntityStateValue() {
        return entityState.getIntValue();
    }

    /**
     * Set the state of the network element mapped to this virtual interface.
     *
     * <p>
     *   This method is called by JAXB.
     * </p>
     *
     * @param st  A numerical representation of the network element state.
     */
    @SuppressWarnings("unused")
    private void setEntityStateValue(int st) {
        this.entityState = VBridge.toVnodeState(st);
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
     *     {@code o} is a {@code VInterface} object.
     *   </li>
     *   <li>
     *     The following values stored in {@code o} are the same as in this
     *     object.
     *     <ul>
     *       <li>
     *         The name of the
     *         {@linkplain <a href="package-summary.html#vInterface">virtual interface</a>}.
     *       </li>
     *       <li>The description of the virtual interface.</li>
     *       <li>Interface enable/disable configuration.</li>
     *       <li>The {@linkplain #getState() state of the virtual interface}.</li>
     *       <li>
     *         The state of the network element mapped to the virtual
     *         interface.
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
        if (!(o instanceof VInterface) || !super.equals(o)) {
            return false;
        }

        VInterface viface = (VInterface)o;
        return (state == viface.state && entityState == viface.entityState &&
                Objects.equals(name, viface.name));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode() ^ state.toString().hashCode() ^
            entityState.toString().hashCode();
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
        StringBuilder builder = new StringBuilder("VInterface[");
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
        builder.append(prefix).append("state=").append(state.toString()).
            append(",entityState=").append(entityState.toString()).append(']');

        return builder.toString();
    }


    // VTNIdentifiable

    /**
     * Return the identifier of this instance.
     *
     * <p>
     *   This class uses the name of the VInterface as the identifier.
     * </p>
     *
     * @return  The name of the VInterface.
     * @since   Lithium
     */
    @Override
    public String getIdentifier() {
        return name;
    }
}
