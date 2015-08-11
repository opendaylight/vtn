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
 * This class describes a flow action that sets the specified ICMP code
 * into the ICMP header in IPv4 packet.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"code": 1
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "seticmpcode")
@XmlAccessorType(XmlAccessType.NONE)
public final class SetIcmpCodeAction extends FlowAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 8269388333320510437L;

    /**
     * ICMP code to be set.
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
    private short  code;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private SetIcmpCodeAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param code  ICMP code to be set.
     */
    public SetIcmpCodeAction(short code) {
        this.code = code;
    }

    /**
     * Return the ICMP code to be set.
     *
     * @return  ICMP code to be set.
     */
    public short getCode() {
        return code;
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

        SetIcmpCodeAction icmpcode = (SetIcmpCodeAction)o;
        return (code == icmpcode.code);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() + ((int)code * 13);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("SetIcmpCodeAction[");
        builder.append("code=").append((int)code).append(']');

        return builder.toString();
    }
}
