/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.internal.LeafBuildHelper;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vterminal.rev150907.vtn.vterminal.info.VterminalConfigBuilder;

/**
 * Helper class for building vterminal-config instance.
 */
public final class VterminalConfigBuildHelper
    extends LeafBuildHelper<VterminalConfig, VterminalConfigBuilder> {
    /**
     * Construct a new instance.
     */
    public VterminalConfigBuildHelper() {
        super(VterminalConfig.class, new VterminalConfigBuilder());
    }
}
