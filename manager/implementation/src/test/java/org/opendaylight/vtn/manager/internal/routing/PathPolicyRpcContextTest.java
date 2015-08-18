/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import java.util.Collections;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.internal.FlowRemover;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.flow.remove.PathPolicyFlowRemover;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link PathPolicyRpcContext}
 */

public class PathPolicyRpcContextTest extends TestBase {
    /**
     * Test case for all instance methods.
     *
     * <ul>
     *   <li>{@link PathPolicyRpcContext#PathPolicyRpcContext(TopologyGraph,Integer)}</li>
     *   <li>{@link PathPolicyRpcContext#getPolicyId()}</li>
     *   <li>{@link PathPolicyRpcContext#onStarted()}</li>
     *   <li>{@link PathPolicyRpcContext#onUpdated()}</li>
     *   <li>{@link PathPolicyRpcContext#getFlowRemover()}</li>
     * </ul>
     */
    @Test
    public void test() {
        VTNManagerProvider provider = Mockito.mock(VTNManagerProvider.class);
        TopologyGraph topo = new TopologyGraph(provider);
        Integer[] policies = {1, 2, 3};

        for (Integer id: policies) {
            PathPolicyRpcContext context = new PathPolicyRpcContext(topo, id);
            assertEquals(id, context.getPolicyId());

            FlowRemover rmv = context.getFlowRemover();
            assertTrue(rmv instanceof PathPolicyFlowRemover);
            PathPolicyFlowRemover remover = (PathPolicyFlowRemover)rmv;
            assertEquals(Collections.singleton(id),
                         remover.getPathPolicyIds());

            context.onStarted();
            topo.updateResolver(id);
            context.onUpdated();
        }
    }
}
