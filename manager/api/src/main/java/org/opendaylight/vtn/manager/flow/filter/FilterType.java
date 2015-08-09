/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow.filter;

import java.io.Serializable;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlSeeAlso;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * {@code FilterType} is an abstract class that describes the type of
 * flow filter.
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "filtertype")
@XmlAccessorType(XmlAccessType.NONE)
@XmlSeeAlso({PassFilter.class, DropFilter.class, RedirectFilter.class})
public abstract class FilterType implements Serializable {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -4140830853020675751L;

    /**
     * Construct a new instance.
     */
    FilterType() {
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
