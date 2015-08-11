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
 * This class describes a flow action that sets the specified ICMP type
 * into the ICMP header in IPv4 packet.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"type": 5
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "seticmptype")
@XmlAccessorType(XmlAccessType.NONE)
public final class SetIcmpTypeAction extends FlowAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 5063088824539412489L;

    /**
     * ICMP type to be set.
     *
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>255</strong>.
     *   </li>
     *   <li>
     *     If this attribute is omitted, it will be treated as
     *     <strong>0</strong> is specified.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private short  type;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private SetIcmpTypeAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param type  ICMP type to be set.
     */
    public SetIcmpTypeAction(short type) {
        this.type = type;
    }

    /**
     * Return the ICMP type to be set.
     *
     * @return  ICMP type to be set.
     */
    public short getType() {
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
        if (!super.equals(o)) {
            return false;
        }

        SetIcmpTypeAction icmptype = (SetIcmpTypeAction)o;
        return (type == icmptype.type);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() + ((int)type * 29);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("SetIcmpTypeAction[");
        builder.append("type=").append((int)type).append(']');

        return builder.toString();
    }
}
