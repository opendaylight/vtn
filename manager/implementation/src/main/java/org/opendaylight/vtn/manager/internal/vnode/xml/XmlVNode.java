/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode.xml;

import java.util.Objects;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodeName;

/**
 * {@code XmlVNode} is a base class for classes that provides XML binding
 * to the data model for virtual node.
 */
@XmlRootElement(name = "vnode")
@XmlAccessorType(XmlAccessType.NONE)
public abstract class XmlVNode {
    /**
     * The name of the virtual node.
     */
    @XmlElement(required = true)
    private String  name;

    /**
     * Constructor only for JAXB.
     */
    protected XmlVNode() {
    }

    /**
     * Construct a new instance.
     *
     * @param vname  A {@link VnodeName} instance that contains the name of the
     *               virtual node.
     */
    protected XmlVNode(VnodeName vname) {
        name = vname.getValue();
    }

    /**
     * Return the name of the virtual node.
     *
     * @return  The name of the virtual node.
     */
    public final String getName() {
        return name;
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (o == null || !getClass().equals(o.getClass())) {
            return false;
        }

        XmlVNode xvn = (XmlVNode)o;
        return name.equals(xvn.name);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(getClass(), name);
    }
}
