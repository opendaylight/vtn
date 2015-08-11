/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.action;

import java.io.Serializable;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * This class describes an abstract information about an action in a flow
 * entry.
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "flowaction")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({
    DropAction.class,
    PopVlanAction.class,
    PushVlanAction.class,
    SetDlDstAction.class,
    SetDlSrcAction.class,
    SetDscpAction.class,
    SetIcmpCodeAction.class,
    SetIcmpTypeAction.class,
    SetInet4DstAction.class,
    SetInet4SrcAction.class,
    SetTpDstAction.class,
    SetTpSrcAction.class,
    SetVlanIdAction.class,
    SetVlanPcpAction.class})
public abstract class FlowAction implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 4165318037172765119L;

    /**
     * Construct a new instance which describes an action in a flow entry.
     */
    FlowAction() {
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
        return (o != null && getClass().equals(o.getClass()));
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return getClass().getName().hashCode();
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        return getClass().getSimpleName();
    }
}
