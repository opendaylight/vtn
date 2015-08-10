/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlValue;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * This class describes an arbitrary long integer value.
 *
 * <p>
 *   This class is used to return an long integer value as a response of
 *   REST API.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"value": 100
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "integer")
@XmlAccessorType(XmlAccessType.NONE)
public class XmlLongInteger {
    /**
     * An arbitrary long integer value.
     *
     * <ul>
     *   <li>
     *     In JSON notation, the value of this element is represented as a
     *     property named <strong>value</strong>.
     *   </li>
     * </ul>
     */
    @XmlValue
    private Long  value;

    /**
     * Private constructor only for JAXB.
     */
    private XmlLongInteger() {
    }

    /**
     * Construct a new instance.
     *
     * @param v  A long integer value.
     */
    public XmlLongInteger(long v) {
        value = Long.valueOf(v);
    }

    /**
     * Construct a new instance.
     *
     * @param v  An integer value.
     */
    public XmlLongInteger(int v) {
        this((long)v);
    }

    /**
     * Return a value configured in this instance.
     *
     * @return  A {@link Long} instance which represents the value.
     *          {@code null} is returned if no value is configured in this
     *          instance.
     */
    public Long getValue() {
        return value;
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
        if (!(o instanceof XmlLongInteger)) {
            return false;
        }

        Long anotherValue = ((XmlLongInteger)o).value;
        if (value == null) {
            return (anotherValue == null);
        }

        return value.equals(anotherValue);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return (value == null) ? 0 : value.hashCode();
    }
}
