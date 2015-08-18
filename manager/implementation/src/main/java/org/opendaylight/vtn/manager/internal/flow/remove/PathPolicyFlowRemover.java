/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.flow.remove;

import java.util.Collections;
import java.util.HashSet;
import java.util.Set;

import org.opendaylight.vtn.manager.internal.util.flow.FlowCache;

/**
 * An implementation of
 * {@link org.opendaylight.vtn.manager.internal.FlowRemover} which removes
 * VTN data flows whose packet route were determined by the specified path
 * policy.
 *
 * <p>
 *   This flow remover affects flow entries in all the VTNs.
 * </p>
 */
public final class PathPolicyFlowRemover extends GlobalScanFlowRemover {
    /**
     * A set of path policy identifiers.
     */
    private final Set<Integer>  policyIds;

    /**
     * Construct a new instance.
     *
     * @param id  The identifier of the path policy.
     */
    public PathPolicyFlowRemover(int id) {
        policyIds = Collections.singleton(Integer.valueOf(id));
    }

    /**
     * Construct a new instance.
     *
     * @param set  A set of path policy identifiers.
     */
    public PathPolicyFlowRemover(Set<Integer> set) {
        policyIds = new HashSet<Integer>(set);
    }

    /**
     * Return a set of the target path policy identifiers.
     *
     * @return  A set of path policy identifiers.
     */
    public Set<Integer> getPathPolicyIds() {
        return policyIds;
    }

    // ScanFlowRemover

    /**
     * {@inheritDoc}
     */
    @Override
    protected boolean select(FlowCache fc) {
        Integer pid = fc.getDataFlow().getPathPolicyId();
        return policyIds.contains(pid);
    }

    // FlowRemover

    /**
     * {@inheritDoc}
     */
    @Override
    public String getDescription() {
        return new StringBuilder("path-policy[").append(policyIds).
            append(']').toString();
    }
}
