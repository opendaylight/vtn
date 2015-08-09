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
 * This class describes a flow action that takes a port number of the transport
 * layer as argument.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"port": 10
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "setport")
@XmlAccessorType(XmlAccessType.NONE)
public abstract class TpPortAction extends FlowAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 5891967599355680390L;

    /**
     * A port number to be set.
     *
     * <ul>
     *   <li>
     *     The range of value that can be specified is from
     *     <strong>0</strong> to <strong>65535</strong>.
     *   </li>
     *   <li>
     *     If this attribute is omitted, it will be treated as
     *     <strong>0</strong> is specified.
     *   </li>
     * </ul>
     */
    @XmlAttribute
    private int  port;

    /**
     * Dummy constructor only for JAXB.
     */
    TpPortAction() {
    }

    /**
     * Construct a new instance.
     *
     * @param port  A port number for the transport layer protocol.
     */
    TpPortAction(int port) {
        this.port = port;
    }

    /**
     * Return a port number configured in this instance.
     *
     * @return  A port number configured in this instance.
     */
    public final int getPort() {
        return port;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public final boolean equals(Object o) {
        if (o == this) {
            return true;
        }
        if (!super.equals(o)) {
            return false;
        }

        TpPortAction tpport = (TpPortAction)o;
        return (port == tpport.port);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public final int hashCode() {
        return super.hashCode() + (port * 41);
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public final String toString() {
        StringBuilder builder = new StringBuilder(getClass().getSimpleName());
        builder.append("[port=").append(port).append(']');

        return builder.toString();
    }
}
