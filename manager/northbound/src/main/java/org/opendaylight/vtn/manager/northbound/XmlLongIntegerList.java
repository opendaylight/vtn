/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.ArrayList;
import java.util.List;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

/**
 * {@code XmlLongIntegerList} class describes a list of arbitrary long
 * integer values.
 *
 * <p>
 *   This class is used to return an information represented by a list of
 *   long integer values as a response of REST API.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"integer": [
 * &nbsp;&nbsp;&nbsp;&nbsp;{ "value": 100 },
 * &nbsp;&nbsp;&nbsp;&nbsp;{ "value": 200 }
 * &nbsp;&nbsp;]
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "integers")
@XmlAccessorType(XmlAccessType.NONE)
public class XmlLongIntegerList {
    /**
     * A list of {@link XmlLongInteger} instances.
     *
     * <ul>
     *   <li>
     *     This element contains 0 or more {@link XmlLongInteger} instances.
     *   </li>
     * </ul>
     */
    @XmlElement(name = "integer")
    private List<XmlLongInteger>  integers;

    /**
     * Default constructor.
     */
    public XmlLongIntegerList() {
    }

    /**
     * Construct a list of long integer values.
     *
     * @param list  A list of long integer values.
     */
    public XmlLongIntegerList(List<? extends Number> list) {
        if (list != null) {
            integers = new ArrayList<XmlLongInteger>(list.size());
            for (Number i: list) {
                integers.add(new XmlLongInteger(i.longValue()));
            }
        }
    }

    /**
     * Return a list of long integer values.
     *
     * @return A list of long integer values.
     */
    List<XmlLongInteger> getIntegers() {
        return integers;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (!(o instanceof XmlLongIntegerList)) {
            return false;
        }

        List<XmlLongInteger> list = ((XmlLongIntegerList)o).integers;
        if (integers == null || integers.isEmpty()) {
            return (list == null || list.isEmpty());
        }

        return integers.equals(list);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (integers != null && !integers.isEmpty()) {
            h = integers.hashCode();
        }

        return h;
    }
}
