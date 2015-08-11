/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * This class describes a flow action that sets the specified VLAN ID into
 * the packet.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"vlan": 10
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "setvlanid")
@XmlAccessorType(XmlAccessType.NONE)
public final class SetVlanIdAction extends FlowAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 1112583053740773108L;

    /**
     * VLAN ID to be set.
     *
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>1</strong> to <strong>4095</strong>.
     *     <ul>
     *       <li>
     *         Note that <strong>0</strong> cannot be specified.
     *         If you need a flow action which removes VLAN tag from packets,
     *         use {@link PopVlanAction} instead.
     *       </li>
     *     </ul>
     *   </li>
     *   <li>
     *     This attribute is mandatory.
     *   </li>
     * </ul>
     */
    @XmlAttribute(required = true)
    private short  vlan;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private SetVlanIdAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param vlan  VLAN ID to be set.
     */
    public SetVlanIdAction(short vlan) {
        this.vlan = vlan;
    }

    /**
     * Return the VLAN ID to be set.
     *
     * @return  VLAN ID to be set.
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
        if (!super.equals(o)) {
            return false;
        }

        SetVlanIdAction setvlan = (SetVlanIdAction)o;
        return (vlan == setvlan.vlan);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() + ((int)vlan * 11);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("SetVlanIdAction[");
        builder.append("vlan=").append((int)vlan).append(']');

        return builder.toString();
    }
}
