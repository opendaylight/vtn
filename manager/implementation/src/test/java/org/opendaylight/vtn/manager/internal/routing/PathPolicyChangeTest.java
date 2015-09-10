/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import static org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtilsTest.PATH_POLICY_MIN;
import static org.opendaylight.vtn.manager.internal.util.pathpolicy.PathPolicyUtilsTest.PATH_POLICY_MAX;

import org.junit.Test;

import org.mockito.Mockito;

import org.opendaylight.vtn.manager.internal.VTNManagerProvider;
import org.opendaylight.vtn.manager.internal.routing.xml.XmlPathPolicy;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicy;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policies.VtnPathPolicyBuilder;

/**
 * JUnit test for {@link PathPolicyChange}
 */
public class PathPolicyChangeTest extends TestBase {
    /**
     * Test case for the following methods.
     *
     * <ul>
     *   <li>{@link PathPolicyChange#onUpdated(Integer,XmlPathPolicy)}</li>
     *   <li>{@link PathPolicyChange#onRemoved(Integer)}</li>
     * </ul>
     */
    @Test
    public void test() {
        VTNManagerProvider provider = Mockito.mock(VTNManagerProvider.class);
        TopologyGraph topo = new TopologyGraph(provider);
        PathPolicyChange change = new PathPolicyChange(topo);
        for (int id = PATH_POLICY_MIN; id <= PATH_POLICY_MAX; id++) {
            Integer pid = Integer.valueOf(id);
            VtnPathPolicy vpp = new VtnPathPolicyBuilder().setId(pid).build();
            XmlPathPolicy xpp = new XmlPathPolicy(vpp);
            assertEquals(true, change.onUpdated(id, xpp));
            assertEquals(false, change.onUpdated(id, xpp));
            assertEquals(false, change.onUpdated(id, xpp));
            change.onRemoved(id);
        }
    }
}
