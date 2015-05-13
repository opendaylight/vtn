/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import java.util.List;

import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.VTenantPath;

import org.opendaylight.vtn.manager.internal.cluster.VBridgeMapPath;
import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;
import org.opendaylight.vtn.manager.internal.util.vnode.VNodeUtils;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.flow.rev150410.vtn.data.flow.common.VirtualRoute;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.tenant.flow.info.VtnDataFlow;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.vtn.data.flow.fields.FlowMapList;

/**
 * An implementation of
 * {@link org.opendaylight.vtn.manager.internal.FlowRemover} which removes
 * VTN data flows related to the specified virtual node.
 *
 * <p>
 *   This flow remover affects flow entries only in the VTN which contains the
 *   specified virtual node.
 * </p>
 */
public final class VNodeFlowRemover extends TenantScanFlowRemover {
    /**
     * A {@link VTenantPath} instance which specifies the virtual node.
     */
    private final VTenantPath  nodePath;

    /**
     * Identifier for the virtual network mapping specified by
     * {@link #nodePath}.
     */
    private final String  mapId;

    /**
     * Construct a new instance.
     *
     * @param path  A path to the virtual node.
     *              Specifying {@code null} results in undefined behavior.
     */
    public VNodeFlowRemover(VTenantPath path) {
        super(path.getTenantName());
        nodePath = path;
        mapId = (path instanceof VBridgeMapPath) ? path.toString() : null;
    }

    // ScanFlowRemover

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean select(FlowCache fc) throws VTNException {
        VtnDataFlow vdf = fc.getDataFlow();
        List<VirtualRoute> vroutes = vdf.getVirtualRoute();
        if (vroutes != null) {
            for (VirtualRoute vr: vroutes) {
                VNodePath vnpath =
                    VNodeUtils.toVNodePath(vr.getVirtualNodePath());
                if (nodePath.contains(vnpath)) {
                    return true;
                }
            }
        }

        if (mapId != null) {
            // Select if the given data flow was established by the specified
            // virtual network mapping.
            List<FlowMapList> maps = vdf.getFlowMapList();
            if (maps != null) {
                for (FlowMapList fml: maps) {
                    if (mapId.equals(fml.getFlowMapId())) {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    // FlowRemover

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        return new StringBuilder("virtual-node[").append(nodePath).
            append(']').toString();
    }
}
