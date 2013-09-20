/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.List;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;

import org.opendaylight.vtn.manager.VTenant;

/**
 * A JAXB class which represents a list of virtual tenant information.
 */
@XmlRootElement(name = "vtns")
@XmlAccessorType(XmlAccessType.NONE)
public class VTenantList {
    /**
     * A list of virtual tenant information.
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
     * Return a list of virtual tenant information.
     *
     * @return A list of virtual tenant information.
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
