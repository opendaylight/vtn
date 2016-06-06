/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.internal.LeafBuildHelper;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.rev150328.vtn.info.VtenantConfigBuilder;

/**
 * Helper class for building vtenant-config instance.
 */
public final class VtenantConfigBuildHelper
    extends LeafBuildHelper<VtenantConfig, VtenantConfigBuilder> {
    /**
     * Construct a new instance.
     */
    public VtenantConfigBuildHelper() {
        super(VtenantConfig.class, new VtenantConfigBuilder());
    }
}
