/*
 * Copyright (c) 2013 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

/**
 * {@code FlowGroupId} describes an identifier of flow group created by the
 * VTN Manager.
 *
 * <p>
 *   {@code FlowGroupId} class extends {@link ClusterEventId} class so that
 *   the VTN manager can distinguish flow groups created by the local cluster
 *   node from a set of flows.
 * </p>
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class FlowGroupId extends ClusterEventId {
    private static final long serialVersionUID = -5792856577909605564L;

    /**
     * Prefix of the flow group name.
     */
    private static final String  NAME_PREFIX = "vtn:";

    /**
     * The name of the virtual tenant to which the flow group belongs.
     */
    private final String  tenantName;

    /**
     * Construct a new flow group ID.
     *
     * <p>
     *   This constructor uses the local cluster node ID as the node ID, and
     *   assigns a new event ID.
     * </p>
     *
     * @param tname  The name of the virtual tenant to which the flow group
     *               belongs. Specifying {@code null} results in undefined
     *               behavior.
     */
    public FlowGroupId(String tname) {
        tenantName = tname;
    }

    /**
     * Construct a new flow group ID.
     *
     * @param node   The node ID.
     * @param event  The event ID.
     * @param tname  The name of the virtual tenant to which the flow group
     *               belongs. Specifying {@code null} results in undefined
     *               behavior.
     */
    public FlowGroupId(long node, long event, String tname) {
        super(node, event);
        tenantName = tname;
    }

    /**
     * Return the name of the virtual tenant to which this flow group belongs.
     *
     * @return  The name of the virtual tenant.
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
        if (!(o instanceof FlowGroupId) || !super.equals(o)) {
            return false;
        }

        FlowGroupId fgid = (FlowGroupId)o;
        return tenantName.equals(fgid.tenantName);
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return super.hashCode() ^ tenantName.hashCode();
    }

    /**
     * Return a string representation of this object.
     *
     * <p>
     *   Node that the returned string by this method will be passed to
     *   forwarding rule manager as flow group name.
     * </p>
     *
     * @return  A string representation of this object.
     */
    @Override
    public String toString() {
        StringBuilder builder = new StringBuilder(NAME_PREFIX);
        builder.append(tenantName).append(SEPARATOR).append(super.toString());
        return builder.toString();
    }
}
