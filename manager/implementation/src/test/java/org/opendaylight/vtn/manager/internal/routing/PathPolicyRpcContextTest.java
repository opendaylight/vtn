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
import org.junit.Assert;
import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.VTNManagerImpl;
import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.PathPolicyFlowSelector;

/**
 * JUnit test for {@link PathPolicyRpcContext}
 */

public class PathPolicyRpcContextTest extends TestBase {

    /**
     * Instance of VTNManagerImpl to perform unit testing.
     */
    VTNManagerImpl vtnImpl = new VTNManagerImpl();
    /**
     * Instance of VTNManagerProvider to perform unit testing.
     */
    VTNManagerProvider vtnProv = vtnImpl.getVTNProvider();
    /**
     * Instance of TopologyGraph to perform unit testing.
     */
    TopologyGraph topology = new TopologyGraph(vtnProv);
    /**
     * Instance of PathPolicyRpcContext to perform unit testing.
     */
    PathPolicyRpcContext path1 = new PathPolicyRpcContext(topology, new Integer(10));
    PathPolicyRpcContext path2 = new PathPolicyRpcContext(topology, new Integer(-10));
    PathPolicyRpcContext path3 = new PathPolicyRpcContext(topology, null);
    PathPolicyRpcContext path4 = null;

    /**
     * Test method for
     * {@link PathPolicyRpcContext#getPolicyId()}.
     */
    @Test
    public void getPolicyIdTest() {
        try {
            assertEquals(new Integer(10), path1.getPolicyId());
            assertEquals(new Integer(-10), path2.getPolicyId());
            assertEquals(null, path3.getPolicyId());
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof NullPointerException);
        }

    }

    /**
     * Test method for
     * {@link PathPolicyRpcContext#onStarted()}
     * {@link PathPolicyRpcContext#onUpdated()}.
     */
    @Test
    public void onStartedAndUpdatedTest() {
        try {

            path1.onStarted();
            path1.onUpdated();
            path4.onStarted();
            path4.onUpdated();
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof NullPointerException);
        }
    }

    /**
     * Test method for
     * {@link PathPolicyRpcContext#getFlowSelector()}
     */
    @Test
    public void getFlowSelectorTest() {
        try {

            assertTrue(path1.getFlowSelector() instanceof PathPolicyFlowSelector);
            assertTrue(path4.getFlowSelector() instanceof PathPolicyFlowSelector);
        } catch (Exception ex) {
            Assert.assertTrue(ex instanceof NullPointerException);
        }
    }
}
