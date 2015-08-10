/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.routing;

import org.junit.Test;


import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.vtn.manager.PathCost;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.SwitchPort;
import org.opendaylight.vtn.manager.internal.TestBase;
import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostBuilder;


/**
 * JUnit test for {@link SetCostTask}.
 */
public class SetCostTaskTest extends TestBase {
    /**
     * Test method for {@link SetCostTask#getPortDesc()}
     */
    @Test
    public void testGetPortDesc() {
        // Create a list of valid port locations.
        List<PortLocation> portLocs = new ArrayList<>();
        List<VtnPortDesc> portDescs = new ArrayList<>();
        try {
            long[] dpids = {Long.MIN_VALUE, -1234567L, -1L, 1L, 0xabcdef1234567L, Long.MAX_VALUE, };
            String[] ports = {null, "1", "10", };
            String[] names = {null, "port-1", "port-10", };

            for (long dpid : dpids) {
                SalNode snode = new SalNode(dpid);
                Node node = snode.getAdNode();
                for (String port : ports) {
                    String p;
                    String type;
                    if (port == null) {
                        p = "";
                        type = null;
                    } else {
                        p = port;
                        type = NodeConnectorIDType.OPENFLOW;
                    }
                    for (String name : names) {
                        String n = (name == null) ? "" : name;
                        String pd = joinStrings(null, null, ",", snode, p, n);
                        VtnPortDesc vdesc = new VtnPortDesc(pd);
                        portDescs.add(vdesc);
                        SwitchPort swport = (port == null && name == null) ? null
                                : new SwitchPort(name, type, port);
                        PortLocation ploc = new PortLocation(node, swport);
                        portLocs.add(ploc);
                    }
                }
            }

            Long[] costs = {null, Long.valueOf(1L), Long.valueOf(2L),
                    Long.valueOf(100000L), Long.valueOf(9999999999L),
                    Long.valueOf(Long.MAX_VALUE - 1L),
                    Long.valueOf(Long.MAX_VALUE), };

            for (Long cost : costs) {
                Long exCost = cost;
                if (cost != null) {
                    Iterator<VtnPortDesc> descIterator = portDescs.iterator();
                    for (PortLocation ploc : portLocs) {
                        PathCost pc = new PathCost(ploc, cost.longValue());
                        VtnPortDesc vdesc = descIterator.next();
                        VtnPathCost vtnPathCost = new VtnPathCostBuilder()
                                .setPortDesc(vdesc).setCost(cost).build();
                        SetCostTask setCostTask = new SetCostTask(1,
                                vtnPathCost);
                        VtnPortDesc result = setCostTask.getPortDesc();
                        VtnPortDesc expected = setCostTask.getDataObject()
                                .getPortDesc();
                        assertEquals(result, expected);
                    }
                }
            }

        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
}
