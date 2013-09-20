/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import javax.xml.bind.annotation.XmlAccessType;
import javax.xml.bind.annotation.XmlAccessorType;
import javax.xml.bind.annotation.XmlAttribute;
import javax.xml.bind.annotation.XmlRootElement;

/**
 * {@code VTenant} class describes information about the virtual tenant.
 */
@XmlRootElement(name = "vtn")
@XmlAccessorType(XmlAccessType.NONE)
public class VTenant extends VTenantConfig {
    private final static long serialVersionUID = 2450381614616453968L;

    /**
     * The name of the tenant.
     */
    @XmlAttribute(required = true)
    private String  name;

    /**
     * Private constructor used for JAXB mapping.
     */
    private VTenant() {
        super(null);
    }

    /**
     * Construct a new tenant instance.
     *
     * @param tenantName  The name of the tenant.
     * @param tconf       Virtual tenant configuration.
     * @throws NullPointerException
     *    {@code tcode} is {@code null}.
     */
    public VTenant(String tenantName, VTenantConfig tconf) {
        super(tconf.getDescription(), tconf.getIdleTimeout(),
              tconf.getHardTimeout());
        name = tenantName;
    }

    /**
     * Return the name of the tenant.
     *
     * @return  The name of the tenant.
     */
    public String getName() {
        return name;
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
        if (!(o instanceof VTenant) || !super.equals(o)) {
            return false;
        }

        VTenant vtenant = (VTenant)o;
        if (name == null) {
            return (vtenant.name == null);
        }

        return name.equals(vtenant.name);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = super.hashCode();
        if (name != null) {
            h ^= name.hashCode();
        }

        return h;
    }

    /**
     * Return a string representation of this object.
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder("VTenant[");
        char sep = 0;
        if (name != null) {
            builder.append("name=").append(name);
            sep = ',';
        }

        String desc = getDescription();
        if (desc != null) {
            if (sep != 0) {
                builder.append(sep);
            }
            builder.append("desc=").append(desc);
            sep = ',';
        }

        int idle = getIdleTimeout();
        if (idle >= 0) {
            if (sep != 0) {
                builder.append(sep);
            }
            builder.append("idleTimeout=").append(idle);
            sep = ',';
        }

        int hard = getHardTimeout();
        if (hard >= 0) {
            if (sep != 0) {
                builder.append(sep);
            }
            builder.append("hardTimeout=").append(hard);
        }
        builder.append(']');

        return builder.toString();
    }
}
