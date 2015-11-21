/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.adsal;

import org.opendaylight.vtn.manager.VBridgePath;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.impl.flow.rev150313.BridgeMapInfo;

/**
 * {@code VBridgeMapPath} class describes the location of the virtual mapping
 * configured to the vBridge.
 */
public abstract class VBridgeMapPath extends VBridgePath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -4610294464507994162L;

    /**
     * Construct a path to the virtual mapping to the vBridge.
     *
     * @param bridgePath  Path to the virtual bridge.
     * @throws NullPointerException  {@code bridgePath} is {@code null}.
     */
    public VBridgeMapPath(VBridgePath bridgePath) {
        super(bridgePath.getTenantName(), bridgePath.getBridgeName());
    }

    /**
     * Return a {@link VBridgePath} instance which represents the location of
     * the vBridge.
     *
     * @return  A {@link VBridgePath} instance.
     */
    public final VBridgePath getBridgePath() {
        return new VBridgePath(getTenantName(), getBridgeName());
    }

    /**
     * Return a {@link BridgeMapInfo} instance which represents the virtual
     * network mapping information.
     *
     * @return  A {@link BridgeMapInfo} instance.
     */
    public abstract BridgeMapInfo getBridgeMapInfo();
}
