/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

/**
 * {@link VTNFlowMatch} implementation which accepts VTN flows which was
 * determined the packet route by the specified path policy.
 */
public class PathPolicyFlowMatch implements VTNFlowMatch {
    /**
     * The identifier of the path policy.
     */
    private final int  policyId;

    /**
     * Construct a new instance.
     *
     * @param id  The identifier of the path policy.
     */
    public PathPolicyFlowMatch(int id) {
        policyId = id;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean accept(VTNFlow vflow) {
        return (vflow.getPathPolicy() == policyId);
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        StringBuilder builder = new StringBuilder("path-policy=");
        return builder.append(policyId).toString();
    }
}
