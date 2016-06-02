/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.internal.LeafBuildHelper;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatus;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vinterface.rev150907.vtn.mappable.vinterface.VinterfaceStatusBuilder;

/**
 * Helper class for building vinterface-status instance.
 */
public final class VinterfaceStatusBuildHelper
    extends LeafBuildHelper<VinterfaceStatus, VinterfaceStatusBuilder> {
    /**
     * Construct a new instance.
     */
    public VinterfaceStatusBuildHelper() {
        super(VinterfaceStatus.class, new VinterfaceStatusBuilder());
    }
}
