/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.controller.sal.action.SetNwTos;

/**
 * This class describes a flow action that sets the specified value into the
 * DSCP field in the IP packet.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"dscp": 2
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "setdscp")
@XmlAccessorType(XmlAccessType.NONE)
public final class SetDscpAction extends FlowAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 7711399852397187977L;

    /**
     * DSCP field value to be set.
     *
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>63</strong>.
     *   </li>
     *   <li>
     *     If this attribute is omitted, it will be treated as
     *     <strong>0</strong> is specified.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private byte  dscp;

    /**
     * Private constructor only for JAXB.
     */
    @SuppressWarnings("unused")
    private SetDscpAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param dscp  DSCP field value to be set.
     */
    public SetDscpAction(byte dscp) {
        this.dscp = dscp;
    }

    /**
     * Construct a new instance.
     *
     * @param act  A SAL action that sets the DSCP field value.
     * @throws NullPointerException
     *    {@code null} is passed to {@code act}.
     */
    public SetDscpAction(SetNwTos act) {
        dscp = (byte)act.getNwTos();
    }

    /**
     * Return the DSCP field value to be set.
     *
     * @return  DSCP field value to be set.
     */
    public byte getDscp() {
        return dscp;
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

        SetDscpAction setdscp = (SetDscpAction)o;
        return (dscp == setdscp.dscp);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() + ((int)dscp * 17);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("SetDscpAction[");
        builder.append("dscp=").append((int)dscp).append(']');

        return builder.toString();
    }
}
