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
 * {@code MacMapPath} class describes fully-qualified name of the MAC mapping
 * applied to the virtual L2 bridge.
 *
 * <p>
 *   Path to the MAC mapping has the same path components as the vBridge path
 *   because only one MAC mapping can be configured to the vBridge.
 * </p>
 * <p>
 *   Although this class is public to other packages, this class does not
 *   provide any API. Applications other than VTN Manager must not use this
 *   class.
 * </p>
 */
public class MacMapPath extends VBridgePath {
    /**
     * Version number for serialization.
     */
    private static final long serialVersionUID = -5241941891686483002L;

    /**
     * Construct a path to the MAC mapping.
     *
     * @param bridgePath  Path to the virtual bridge.
     * @throws NullPointerException  {@code bridgePath} is {@code null}.
     */
    public MacMapPath(VBridgePath bridgePath) {
        super(bridgePath.getTenantName(), bridgePath.getBridgeName());
    }
}
