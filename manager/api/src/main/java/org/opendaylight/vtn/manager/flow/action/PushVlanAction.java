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

import org.opendaylight.controller.sal.utils.EtherTypes;

/**
 * This class describes a flow action that adds a VLAN tag to the packet.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"type": 33024
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "pushvlan")
@XmlAccessorType(XmlAccessType.NONE)
public final class PushVlanAction extends FlowAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 6991399341142137061L;

    /**
     * Ethernet type of a new VLAN tag.
     *
     * <ul>
     *   <li>
     *     The value must be 33024 (0x8100: 802.1Q) or 34984 (0x88a8: 802.1ad).
     *   </li>
     *   <li>
     *     This attribute is mandatory.
     *   </li>
     * </ul>
     */
    @XmlAttribute(required = true)
    private int  type;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private PushVlanAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param type  Ethernet type of a new VLAN tag.
     */
    public PushVlanAction(int type) {
        this.type = type;
    }

    /**
     * Construct a new instance.
     *
     * @param type  An {@link EtherTypes} instance which represents the
     *              ethernet type.
     * @throws NullPointerException
     *    {@code null} is passed to {@code type}.
     */
    public PushVlanAction(EtherTypes type) {
        this.type = type.intValue();
    }

    /**
     * Return the Ethernet type of a new VLAN tag.
     *
     * @return  Ethernet type.
     */
    public int getType() {
        return type;
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
        if (!(o instanceof PushVlanAction)) {
            return false;
        }

        PushVlanAction pv = (PushVlanAction)o;
        return (type == pv.type);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return PushVlanAction.class.getName().hashCode() ^ type;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("PushVlanAction[");
        return builder.append("type=").append(Integer.toHexString(type)).
            append(']').toString();
    }
}
