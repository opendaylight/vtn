/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.List;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import com.fasterxml.jackson.databind.annotation.JsonSerialize;

import org.opendaylight.vtn.manager.flow.filter.FlowFilter;

/**
 * {@code FlowFilterList} class describes a list of flow filter information.
 *
 * <p>
 *   This class is used to return a list of flow filter information to
 *   REST client.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"flowfilter": [
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"index": 10,
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"condition": "flowcond_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"filterType": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"redirect": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"destination": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"bridge": "vbridge_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"interface": "if_2"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"output": false
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"actions": [
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{"dlsrc": {"address": "f0:f1:f2:f3:f4:f5"}},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{"vlanpcp": {"priority": 7}}
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;]
 * &nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"index": 20,
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"condition": "flowcond_2",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"filterType": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"pass": {}
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"actions": [
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;{"dscp": {"dscp": 10}}
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;]
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;]
 * }</pre>
 *
 * @since  Helium
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "flowfilters")
@XmlAccessorType(XmlAccessType.NONE)
public class FlowFilterList {
    /**
     * A list of {@link FlowFilter} instances.
     *
     * <ul>
     *   <li>
     *     This element contains 0 or more {@link FlowFilter} instances
     *     which represent flow filter information.
     *   </li>
     * </ul>
     */
    @XmlElement(name = "flowfilter")
    private List<FlowFilter>  filters;

    /**
     * Default constructor.
     */
    public FlowFilterList() {
    }

    /**
     * Construct a list of flow filters.
     *
     * @param list  A list of flow filter information.
     */
    public FlowFilterList(List<FlowFilter> list) {
        filters = list;
    }

    /**
     * Return a list of flow filter information.
     *
     * @return A list of flow filter information.
     */
    List<FlowFilter> getFilters() {
        return filters;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (!(o instanceof FlowFilterList)) {
            return false;
        }

        List<FlowFilter> list = ((FlowFilterList)o).filters;
        if (filters == null || filters.isEmpty()) {
            return (list == null || list.isEmpty());
        }

        return filters.equals(list);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (filters != null && !filters.isEmpty()) {
            h ^= filters.hashCode();
        }

        return h;
    }
}
