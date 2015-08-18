/*
 * Copyright (c) 2014, 2015 NEC Corporation. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.flow;

import java.util.HashSet;
import java.util.Set;

import org.junit.Test;

import org.opendaylight.vtn.manager.DataLinkHost;
import org.opendaylight.vtn.manager.SwitchPort;

import org.opendaylight.controller.sal.core.Node;

import org.opendaylight.vtn.manager.TestBase;

/**
 * JUnit test for {@link DataFlowFilter}.
 */
public class DataFlowFilterTest extends TestBase {
    /**
     * Test case for {@link DataFlowFilter} instance methods.
     */
    @Test
    public void testDataFlowFilter() {
        DataFlowFilter df = new DataFlowFilter();
        assertTrue(df.isEmpty());

        HashSet<DataLinkHost> hostSet = new HashSet<DataLinkHost>();
        for (Set<DataLinkHost> set: createDataLinkHostSet(5, false)) {
            hostSet.addAll(set);
        }
        hostSet.add(null);

        DataLinkHost curHost = null;
        Node curNode = null;
        SwitchPort curPort = null;
        for (DataLinkHost host: hostSet) {
            assertSame(df, df.setSourceHost(host));
            curHost = host;
            assertEquals(curHost == null && curNode == null && curPort == null,
                         df.isEmpty());
            assertEquals(curHost, df.getSourceHost());
            assertEquals(curNode, df.getNode());
            assertEquals(curPort, df.getSwitchPort());

            for (Node node: createNodes(5)) {
                assertSame(df, df.setNode(node));
                curNode = node;
                assertEquals(curHost == null && curNode == null &&
                             curPort == null, df.isEmpty());
                assertEquals(curHost, df.getSourceHost());
                assertEquals(curNode, df.getNode());
                assertEquals(curPort, df.getSwitchPort());

                for (SwitchPort port: createSwitchPorts(5)) {
                    assertSame(df, df.setSwitchPort(port));
                    curPort = port;
                    assertEquals(curHost == null && curNode == null &&
                                 curPort == null, df.isEmpty());
                    assertEquals(curHost, df.getSourceHost());
                    assertEquals(curNode, df.getNode());
                    assertEquals(curPort, df.getSwitchPort());
                }
            }
        }

        df.setSourceHost(null).setNode(null).setSwitchPort(null);
        assertTrue(df.isEmpty());
        assertEquals(null, df.getSourceHost());
        assertEquals(null, df.getNode());
        assertEquals(null, df.getSwitchPort());
    }
}
