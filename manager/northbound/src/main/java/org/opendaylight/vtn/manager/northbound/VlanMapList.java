/*
 * Copyright (c) 2013, 2015 NEC Corporation.  All rights reserved.
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

import org.opendaylight.vtn.manager.VlanMap;

/**
 * {@code VlanMapList} class describes a list of VLAN mapping information.
 *
 * <p>
 *   This class is used to return a list of VLAN mapping information to
 *   REST client.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"vlanmap": [
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "OF-00:00:00:00:00:00:00:03.0",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"vlan": "0",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"node": {
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"type": "OF",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "00:00:00:00:00:00:00:03"
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"id": "ANY.7",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"vlan": "7"
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;]
 * }</pre>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vlanmaps")
@XmlAccessorType(XmlAccessType.NONE)
public class VlanMapList {
    /**
     * A list of {@link VlanMap} instances.
     *
     * <ul>
     *   <li>
     *     This element contains 0 or more {@link VlanMap} instances which
     *     represent VLAN mapping information.
     *   </li>
     * </ul>
     */
    @XmlElement(name = "vlanmap")
    private List<VlanMap>  vmapList;

    /**
     * Default constructor.
     */
    public VlanMapList() {
    }

    /**
     * Construct a VLAN mapping list.
     *
     * @param list  A list of VLAN mapping information.
     */
    public VlanMapList(List<VlanMap> list) {
        vmapList = list;
    }

    /**
     * Return a list of VLAN mapping information.
     *
     * @return A list of VLAN mapping information.
     */
    List<VlanMap> getList() {
        return vmapList;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (!(o instanceof VlanMapList)) {
            return false;
        }

        List<VlanMap> list = ((VlanMapList)o).vmapList;
        if (vmapList == null || vmapList.isEmpty()) {
            return (list == null || list.isEmpty());
        }

        return vmapList.equals(list);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (vmapList != null && !vmapList.isEmpty()) {
            h += vmapList.hashCode();
        }

        return h;
    }
}
