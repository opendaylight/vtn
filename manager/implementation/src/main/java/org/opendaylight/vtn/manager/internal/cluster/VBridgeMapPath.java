/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.cluster;

import org.opendaylight.vtn.manager.VBridgePath;

/**
 * {@code VBridgeMapPath} class describes the location of the virtual mapping
 * configured to the vBridge.
 */
public abstract class VBridgeMapPath extends VBridgePath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = 3196286445070810527L;

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
}
