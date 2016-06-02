/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.internal.LeafBuildHelper;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.vbridge.info.VbridgeConfigBuilder;

/**
 * Helper class for building vbridge-config instance.
 */
public final class VbridgeConfigBuildHelper
    extends LeafBuildHelper<VbridgeConfig, VbridgeConfigBuilder> {
    /**
     * Construct a new instance.
     */
    public VbridgeConfigBuildHelper() {
        super(VbridgeConfig.class, new VbridgeConfigBuilder());
    }
}
