/*
 * Copyright (c) 2015 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.junit.Test;

import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;

/**
 * JUnit test for {@link PathPolicyChange}
 */
public class PathPolicyChangeTest extends TestBase {

    /**
     * Test case for onUpdated and onRemoved methods.
     */
    @Test
    public void testSetter() {
        VTNManagerImpl vtnmangerimpl = new VTNManagerImpl();
        VTNManagerProvider impl = vtnmangerimpl.getVTNProvider();
        TopologyGraph topo = new TopologyGraph(impl);
        PathPolicyChange policy = new PathPolicyChange(topo);
        for (Integer in: createIntegers(0, 10)) {
            for (PathPolicy pathpolicy: createPathPolicy()) {
                try {
                    policy.onUpdated(in, pathpolicy);
                } catch (Exception ex) {
                    assertEquals(null, ex.getLocalizedMessage());
                }
            }
        }
        for (Integer in: createIntegers(0, 10)) {
            try {
                policy.onRemoved(in);
            } catch (Exception ex) {
                assertEquals(null, ex.getLocalizedMessage());
            }
        }
        try {
            policy.onRemoved(0);
        } catch (Exception ex) {
            assertEquals(null, ex.getLocalizedMessage());
        }
    }

}
