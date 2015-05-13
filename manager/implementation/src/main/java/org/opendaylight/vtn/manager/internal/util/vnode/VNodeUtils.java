/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.vnode;

import org.opendaylight.vtn.manager.VBridgeIfPath;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.VNodeLocation;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VTerminalIfPath;
import org.opendaylight.vtn.manager.VTerminalPath;

import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePath;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.virtual.route.info.VirtualNodePathBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VnodePathFields;

/**
 * {@code VNodeUtils} class is a collection of utility class methods for
 * for virtual node handling.
 */
public final class VNodeUtils {
    /**
     * Private constructor that protects this class from instantiating.
     */
    private VNodeUtils() {}

    /**
     * Convert the given {@link VNodePath} instance into a
     * {@link VirtualNodePath} instance.
     *
     * @param path  A {@link VNodePath} instance to be converted.
     * @return  A {@link VirtualNodePath} instance.
     *          {@code null} if {@code path} is {@code null}.
     */
    public static VirtualNodePath toVirtualNodePath(VNodePath path) {
        if (path == null) {
            return null;
        }

        VNodeLocation vloc = path.toVNodeLocation();
        return new VirtualNodePathBuilder().
            setTenantName(vloc.getTenantName()).
            setBridgeName(vloc.getBridgeName()).
            setRouterName(vloc.getRouterName()).
            setTerminalName(vloc.getTerminalName()).
            setInterfaceName(vloc.getInterfaceName()).
            build();
    }

    /**
     * Convert the given {@link VnodePathFields} into a {@link VNodePath}
     * instance.
     *
     * @param vpath  A {@link VnodePathFields} instance to be converted.
     * @return  A {@link VNodePath} instance if {@code vpath} is not
     *          {@code null}. {@code null} if {@code vpath} is {@code null}.
     * @throws RpcException
     *    Unable to convert the given instance.
     */
    public static VNodePath toVNodePath(VnodePathFields vpath)
        throws RpcException {
        if (vpath == null) {
            return null;
        }

        String tname = vpath.getTenantName();
        String ifname = vpath.getInterfaceName();
        String bname = vpath.getBridgeName();
        if (bname != null) {
            return (ifname == null)
                ? new VBridgePath(tname, bname)
                : new VBridgeIfPath(tname, bname, ifname);
        }

        String vtname = vpath.getTerminalName();
        if (vtname != null) {
            return (ifname == null)
                ? new VTerminalPath(tname, vtname)
                : new VTerminalIfPath(tname, vtname, ifname);
        }

        throw RpcException.getBadArgumentException(
            "Unexpected virtual node path: " + vpath);
    }
}
