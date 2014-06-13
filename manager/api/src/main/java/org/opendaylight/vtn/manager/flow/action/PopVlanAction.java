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
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * This class describes a flow action that strips the outermost VLAN tag.
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">{}</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "popvlan")
@XmlAccessorType(XmlAccessType.NONE)
public final class PopVlanAction extends FlowAction {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 251560461197844528L;

    /**
     * Construct a new instance.
     */
    public PopVlanAction() {
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        return (o == this || (o instanceof PopVlanAction));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return PopVlanAction.class.hashCode();
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        return "PopVlanAction";
    }
}
