/*
 * Copyright (c) 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.flow;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import org.opendaylight.vtn.manager.internal.util.MiscUtils;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeIdentifier;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.VirtualRouteReason;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.common.VirtualRoute;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.common.VirtualRouteBuilder;

/**
 * {@code VNodeHop} describes a hop in the virtual packet routing within the
 * VTN.
 *
 * <p>
 *   The route of the packet from the source to the destination virtual node
 *   is represented by a sequence of {@code VNodeHop} instances.
 * </p>
 */
public final class VNodeHop {
    /**
     * A {@link VNodeIdentifier} which represents the location of the
     * virtual node inside the VTN.
     *
     * <p>
     *   Note that this element is omitted if this instance indicates that
     *   the data flow is terminated without packet forwarding.
     * </p>
     */
    private VNodeIdentifier<?>  path;

    /**
     * The reason why the packet is forwarded to the virtual node.
     */
    private VirtualRouteReason  reason;

    /**
     * Convert a list of {@link VNodeHop} instances into a list of
     * {@link VirtualRoute} instances.
     *
     * @param vhops  A list of {@link VNodeHop} instances.
     * @return  A list of {@link VirtualRoute} instances if the given list is
     *          not empty. {@code null} if empty.
     */
    public static List<VirtualRoute> toVirtualRouteList(List<VNodeHop> vhops) {
        List<VirtualRoute> list;

        if (MiscUtils.isEmpty(vhops)) {
            list = null;
        } else {
            list = new ArrayList<>(vhops.size());
            int order = MiscUtils.ORDER_MIN;
            for (VNodeHop vhop: vhops) {
                VirtualRoute vroute = vhop.toVirtualRouteBuilder().
                    setOrder(order).
                    build();
                list.add(vroute);
                order++;
            }
        }

        return list;
    }

    /**
     * Construct a new instance which indicates the data flow is terminated.
     */
    public VNodeHop() {
    }

    /**
     * Construct a new instance.
     *
     * @param ident   A {@link VNodeIdentifier} instance that specifies
     *                the location of the virtual node inside the VTN.
     * @param res     A {@link VirtualRouteReason} instance that represents
     *                the reason why the packet is forwarded.
     */
    public VNodeHop(VNodeIdentifier<?> ident, VirtualRouteReason res) {
        path = ident;
        reason = res;
    }

    /**
     * Return the location of the virtual node.
     *
     * @return  A {@link VNodeIdentifier} instance that represents the
     *          location of the virtual node.
     *          {@code null} is returned if this instance indicates that the
     *          data flow is terminated without packet forwarding.
     */
    public VNodeIdentifier<?> getPath() {
        return path;
    }

    /**
     * Return the reason why the packet is forwarded to the virtual node.
     *
     * @return  A {@link VirtualRouteReason} instance that represents the
     *          reason why the packet is forwarded.
     *          {@code null} is returned if this instance indicates that the
     *          data flow is terminated without packet forwarding.
     */
    public VirtualRouteReason getReason() {
        return reason;
    }

    /**
     * Convert this instance into a {@link VirtualRouteBuilder} instance.
     *
     * @return  A {@link VirtualRouteBuilder} instance.
     */
    public VirtualRouteBuilder toVirtualRouteBuilder() {
        VirtualRouteBuilder builder = new VirtualRouteBuilder().
            setReason(reason);
        if (path != null) {
            builder.setVirtualNodePath(path.getVirtualNodePath());
        }

        return builder;
    }

    // Object

    /**
     * Determine whether the given object is identical to this object.
     *
     * @param o  An object to be compared.
     * @return   {@code true} if identical. Otherwise {@code false}.
     */
    @Override
    public boolean equals(Object o) {
        boolean ret = (o == this);
        if (!ret && o != null && getClass().equals(o.getClass())) {
            VNodeHop vhop = (VNodeHop)o;
            ret = (Objects.equals(path, vhop.path) &&
                   Objects.equals(reason, vhop.reason));
        }

        return ret;
    }

    /**
     * Return the hash code of this object.
     *
     * @return  The hash code.
     */
    @Override
    public int hashCode() {
        return Objects.hash(getClass(), path, reason);
    }
}
