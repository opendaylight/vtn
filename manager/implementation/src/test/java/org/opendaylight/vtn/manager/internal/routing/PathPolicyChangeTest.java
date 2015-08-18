/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.PathPolicy;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;

import org.opendaylight.vtn.manager.internal.TestBase;

/**
 * JUnit test for {@link PathPolicyChange}
 */
public class PathPolicyChangeTest extends TestBase {
    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link PathPolicyChange#onUpdated(Integer,PathPolicy)}</li>
     *   <li>{@link PathPolicyChange#onRemoved(Integer)}</li>
     * </ul>
     */
    @Test
    public void test() {
        VTNManagerProvider provider = Mockito.mock(VTNManagerProvider.class);
        TopologyGraph topo = new TopologyGraph(provider);
        PathPolicyChange change = new PathPolicyChange(topo);
        Integer[] policies = {1, 2, 3};
        for (Integer id: policies) {
            PathPolicy pp = new PathPolicy(id, 0L, null);
            assertEquals(true, change.onUpdated(id, pp));
            assertEquals(false, change.onUpdated(id, pp));
            assertEquals(false, change.onUpdated(id, pp));
            change.onRemoved(id);
        }
    }
}
