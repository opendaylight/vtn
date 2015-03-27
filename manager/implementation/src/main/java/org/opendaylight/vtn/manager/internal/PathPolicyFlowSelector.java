/*
 * Copyright (c) 2014-2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal;

import java.util.Collections;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.cluster.VTNFlow;

/**
 * {@link FlowSelector} implementation which accepts VTN flows which was
 * determined the packet route by the specified path policy.
 */
public class PathPolicyFlowSelector implements FlowSelector {
    /**
     * A set of path policy identifiers.
     */
    private final Set<Integer>  policyIds;

    /**
     * Construct a new instance.
     *
     * @param id  The identifier of the path policy.
     */
    public PathPolicyFlowSelector(int id) {
        policyIds = Collections.singleton(Integer.valueOf(id));
    }

    /**
     * Construct a new instance.
     *
     * @param set  A set of path policy identifiers.
     */
    public PathPolicyFlowSelector(Set<Integer> set) {
        policyIds = Collections.unmodifiableSet(set);
    }

    // FlowSelector

    /**
     * {@inheritDoc}
     */
    @Override
    public boolean accept(VTNFlow vflow) {
        return policyIds.contains(vflow.getPathPolicy());
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        StringBuilder builder = new StringBuilder("path-policy=");
        return builder.append(policyIds).toString();
    }
}
