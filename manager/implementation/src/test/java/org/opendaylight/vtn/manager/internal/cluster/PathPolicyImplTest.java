/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */
package org.opendaylight.vtn.manager.internal.cluster;

import org.junit.Test;

import org.opendaylight.vtn.manager.PathPolicy;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.VTNException;
import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.controller.sal.core.NodeConnector;

/**
 * JUnit test for {@link PathPolicyImpl}.
 */

public class PathPolicyImplTest extends TestBase {

    /**
     * Testing the methods in {@link PathPolicyImpl}.
     */
    @Test
    public void testGetter() {
        try {
            for (int idx : INDEX_ARRAY) {
                for (PathPolicy polcy : createPathPolicy()) {
                    PathPolicyImpl policyImpl = new PathPolicyImpl(idx, polcy);

                    assertNotNull(policyImpl.getPathPolicy());
                    assertNotNull(policyImpl.getPolicyId());
                    assertNotNull(policyImpl.hashCode());
                    assertTrue(policyImpl.equals(policyImpl));
                    assertFalse(policyImpl.equals(polcy));
                    assertFalse(policyImpl.equals(null));

                    for (PortLocation location : createPortLocation()) {
                        for (long cost : COST) {
                            policyImpl.setPathCost(location, cost);
                            assertNotNull(policyImpl.getPathCost(location));
                            assertNotNull(policyImpl.removePathCost(location));
                        }
                    }

                    for (VTNManagerImpl impl : createVtnManagerImplobj()) {
                        for (NodeConnector connector : createNodeConnectors(5)) {
                            assertNotNull(policyImpl.getCost(impl, connector));
                            assertNotNull(policyImpl.saveConfig(impl));
                            policyImpl.destroy(impl);
                        }
                    }
                }
            }
        } catch (VTNException | NullPointerException ex) {
            //ex.printStackTrace();
        }
    }
}
