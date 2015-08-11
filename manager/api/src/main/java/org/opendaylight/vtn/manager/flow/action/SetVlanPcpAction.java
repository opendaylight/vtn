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
 * This class describes a flow action that sets the specified VLAN priority
 * into the packet.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"priority": 1
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "setvlanpcp")
@XmlAccessorType(XmlAccessType.NONE)
public final class SetVlanPcpAction extends FlowAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -4917859306147210865L;

    /**
     * VLAN priority to be set.
     *
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>7</strong>.
     *   </li>
     *   <li>
     *     If this attribute is omitted, it will be treated as
     *     <strong>0</strong> is specified.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private byte  priority;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private SetVlanPcpAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param pri  VLAN priority to be set.
     */
    public SetVlanPcpAction(byte pri) {
        priority = pri;
    }

    /**
     * Return the VLAN priority to be set.
     *
     * @return  VLAN priority to be set.
     */
    public byte getPriority() {
        return priority;
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

        SetVlanPcpAction setvlanpcp = (SetVlanPcpAction)o;
        return (priority == setvlanpcp.priority);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() + ((int)priority * 37);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("SetVlanPcpAction[");
        builder.append("priority=").append((int)priority).append(']');

        return builder.toString();
    }
}
