/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.internal.LeafBuildHelper;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.vinterface.info.VinterfaceConfigBuilder;

/**
 * Helper class for building vinterface-config instance.
 */
public final class VinterfaceConfigBuildHelper
    extends LeafBuildHelper<VinterfaceConfig, VinterfaceConfigBuilder> {
    /**
     * Construct a new instance.
     */
    public VinterfaceConfigBuildHelper() {
        super(VinterfaceConfig.class, new VinterfaceConfigBuilder());
    }
}
