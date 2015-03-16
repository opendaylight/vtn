/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.VTenantPath;
import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

/**
 * {@link FlowSelector} implementation which accepts VTN flows which depend
 * on the specified virtual node path.
 */
public class PathFlowSelector implements FlowSelector {
    /**
     * A {@link VTenantPath} instance which specifies the virtual node.
     */
    private final VTenantPath  nodePath;

    /**
     * Construct a new instance.
     *
     * @param path  A path to the virtual node.
     *              Specifying {@code null} results in undefined behavior.
     */
    public PathFlowSelector(VTenantPath path) {
        nodePath = path;
    }

    // FlowSelector

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean accept(VTNFlow vflow) {
        return vflow.dependsOn(nodePath);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        StringBuilder builder = new StringBuilder("path=");
        return builder.append(nodePath).toString();
    }
}
