/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.flow.remove.PathPolicyFlowRemover;

/**
 * {@code PathPolicyRpcContext} describes a runtime context for updating
 * path policy configuration.
 */
final class PathPolicyRpcContext {
    /**
     * The identifier of the target path policy.
     */
    private final Integer  policyId;

    /**
     * The network topology graph.
     */
    private final TopologyGraph  topology;

    /**
     * Revision number of the path policy.
     */
    private Long  revision;

    /**
     * Construct a new instance.
     *
     * @param topo  The network topology graph.
     * @param id    The identifier of the target path policy.
     */
    PathPolicyRpcContext(TopologyGraph topo, Integer id) {
        policyId = id;
        topology = topo;
    }

    /**
     * Return the identifier of the target path policy.
     *
     * @return  The identifier of the target path policy.
     */
    Integer getPolicyId() {
        return policyId;
    }

    /**
     * Invoked when the MD-SAL datastore transaction has started.
     */
    void onStarted() {
        // Preserve current revision of the route resolver associated with
        // the target path policy.
        revision = topology.getRevision(policyId);
    }

    /**
     * Invoked when the target path policy has been updated.
     */
    void onUpdated() {
        // Wait for the route resolver to be updated.
        topology.awaitRevisionUpdated(policyId, revision);
    }

    /**
     * Return a {@link FlowRemover} instance that removes flow entries
     * corresponding to the target path policy.
     *
     * @return  A {@link FlowRemover} instance.
     */
    FlowRemover getFlowRemover() {
        return new PathPolicyFlowRemover(policyId.intValue());
    }
}
