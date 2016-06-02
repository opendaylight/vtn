/*
 * Copyright (c) 2016 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.vnode;

import org.opendaylight.vtn.manager.internal.KeyedLeafBuildHelper;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.bridge.status.FaultedPaths;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.bridge.status.FaultedPathsBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.vbridge.rev150907.vtn.bridge.status.FaultedPathsKey;

/**
 * Helper class for building faulted-paths instance.
 */
public final class FaultedPathsBuildHelper
    extends KeyedLeafBuildHelper<FaultedPaths, FaultedPathsBuilder,
                                 FaultedPathsKey> {
    /**
     * Construct a new instance.
     *
     * @param key  A {@link FaultedPathsKey} instance.
     */
    public FaultedPathsBuildHelper(FaultedPathsKey key) {
        super(FaultedPaths.class,
              new FaultedPathsBuilder().setKey(key).
              setSource(key.getSource()).
              setDestination(key.getDestination()));
    }
}
