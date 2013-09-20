/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager;

import java.io.Serializable;

/**
 * {@code VTenantPath} class describes fully-qualified name of the virtual
 * tenant in the container.
 */
public class VTenantPath implements Serializable {
    private static final long serialVersionUID = -4898647140177601483L;

    /**
     * The name of the tenant.
     */
    private final String  tenantName;

    /**
     * Construct a path to the virtual tenant.
     *
     * @param tenantName  The name of the tenant.
     */
    public VTenantPath(String tenantName) {
        this.tenantName = tenantName;
    }

    /**
     * Return the name of the tenant.
     *
     * @return  The name of the tenant.
     */
    public String getTenantName() {
        return tenantName;
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
        if (!(o instanceof VTenantPath)) {
            return false;
        }

        VTenantPath path = (VTenantPath)o;
        if (tenantName == null) {
            return (path.tenantName == null);
        }

        return tenantName.equals(path.tenantName);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        int h = 0;
        if (tenantName != null) {
            h ^= tenantName.hashCode();
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
        return (tenantName == null) ? "<null>" : tenantName;
    }
}
