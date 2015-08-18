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

import org.opendaylight.vtn.manager.VTenant;

/**
 * {@code VTenantList} class describes a list of VTN (virtual tenant network)
 * information.
 *
 * <p>
 *   This class is used to return a list of VTN information to REST client.
 * </p>
 *
 * <h4>Example JSON</h4>
 * <pre class="prettyprint lang-json">
 * {
 * &nbsp;&nbsp;"vtn": [
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"name": "vtn_1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"description": "Description about VTN 1",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"idleTimeout": "300",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"hardTimeout": "0"
 * &nbsp;&nbsp;&nbsp;&nbsp;},
 * &nbsp;&nbsp;&nbsp;&nbsp;{
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"name": "vtn_2",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"idleTimeout": "600",
 * &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;"hardTimeout": "1000"
 * &nbsp;&nbsp;&nbsp;&nbsp;}
 * &nbsp;&nbsp;]
 * }</pre>
 */
@JsonSerialize(include = JsonSerialize.Inclusion.NON_NULL)
@XmlRootElement(name = "vtns")
@XmlAccessorType(XmlAccessType.NONE)
public class VTenantList {
    /**
     * A list of {@link VTenant} instances.
     *
     * <ul>
     *   <li>
     *     This element contains 0 or more {@link VTenant} instances which
     *     represent VTN information.
     *   </li>
     * </ul>
     */
    @XmlElement(name = "vtn")
    private List<VTenant>  tenantList;

    /**
     * Default constructor.
     */
    public VTenantList() {
    }

    /**
     * Construct a tenant list.
     *
     * @param list  A list of tenant information.
     */
    public VTenantList(List<VTenant> list) {
        tenantList = list;
    }

    /**
     * Return a list of VTN information.
     *
     * @return A list of VTN information.
     */
    List<VTenant> getList() {
        return tenantList;
    }

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        if (!(o instanceof VTenantList)) {
            return false;
        }

        List<VTenant> list = ((VTenantList)o).tenantList;
        if (tenantList == null || tenantList.isEmpty()) {
            return (list == null || list.isEmpty());
        }

        return tenantList.equals(list);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (tenantList != null && !tenantList.isEmpty()) {
            h ^= tenantList.hashCode();
        }

        return h;
    }
}
