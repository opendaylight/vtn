/*
 * Copyright (c) 2014 NEC Corporation
 * All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this
 * distribution, and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.northbound;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;

import org.junit.Test;

import org.opendaylight.vtn.manager.flow.DataFlow;
import org.opendaylight.vtn.manager.VBridgePath;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.VNodePath;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector;
import org.opendaylight.controller.sal.utils.NodeConnectorCreator;
import org.opendaylight.controller.sal.utils.NodeCreator;

/**
 * JUnit test for {@link DataFlowList}.
 */
public class DataFlowListTest extends TestBase {
    /**
     * Test case for getter methods.
     */
    @Test
    public void testGetter() {
        List<DataFlow> list = new ArrayList<DataFlow>();
        short ids = 123;
        long created = 14374808;
        short idle = 300;
        short hard = 0;
        String string1 = "tuuid";
        String string2 = "uuid";
        VNodePath inpath = new VBridgePath(string1, string2);
        Node node = NodeCreator.createOFNode(0L);
        NodeConnector nc = NodeConnectorCreator.
            createOFNodeConnector((short)0, node);
        String name = "data";
        VNodePath outpath = new VBridgePath(string1, string2);
        PortLocation inport = new PortLocation(nc, name);
        PortLocation outport = new PortLocation(nc, name);
        DataFlow succ = new DataFlow(ids, created, idle, hard, inpath, inport,
                                     outpath, outport);
        list.add(succ);

        // null list.
        DataFlowList nullList = new DataFlowList(null);
        assertNull(nullList.getDataFlows());

        DataFlowList list1 = new DataFlowList(list);
        assertEquals(list, list1.getDataFlows());
    }

    /**
     * Test case for {@link DataFlowList#equals(Object)} and
     * {@link DataFlowList#hashCode()}.
     */
    @Test
    public void testEquals() {
        HashSet<Object> set = new HashSet<Object>();

        // null list.
        DataFlowList nullList = new DataFlowList(null);
        testEquals(set, nullList, new DataFlowList(null));

        // Empty list should be treated as null list.
        DataFlowList emptyList = new DataFlowList(new ArrayList<DataFlow>());
        assertEquals(nullList, emptyList);
        assertEquals(nullList.hashCode(), emptyList.hashCode());
        assertFalse(set.add(emptyList));
        List<DataFlow> list1 = new ArrayList<DataFlow>();
        List<DataFlow> list2 = new ArrayList<DataFlow>();
        short ids = 123;
        long created = 14374808;
        short idle = 300;
        short hard = 0;
        String string1 = "tuuid";
        String string2 = "uuid";
        VNodePath inpath = new VBridgePath(string1, string2);
        Node node = NodeCreator.createOFNode(0L);
        NodeConnector nc = NodeConnectorCreator.
            createOFNodeConnector((short)0, node);
        String name = "data";
        VNodePath outpath = new VBridgePath(string1, string2);
        PortLocation inport = new PortLocation(nc, name);
        PortLocation outport = new PortLocation(nc, name);
        DataFlow succ = new DataFlow(ids, created, idle, hard, inpath, inport,
                                     outpath, outport);
        list1.add(succ);
        list2.add(succ);
        DataFlowList df1 = new DataFlowList(list1);
        DataFlowList df2 = new DataFlowList(list2);
        testEquals(set, df1, df2);
    }

    /**
     * Ensure that {@link DataFlowList} is mapped to XML root element.
     */
    @Test
    public void testJAXB() {
        // null list.
        DataFlowList dflowList = new DataFlowList(null);
        String rootName = "dataflows";
        jaxbTest(dflowList, rootName);

        // Empty list.
        List<DataFlow> list = new ArrayList<DataFlow>();
        dflowList = new DataFlowList(list);
        jaxbTest(dflowList, rootName);
        short ids = 123;
        long created = 14374808;
        short idle = 300;
        short hard = 0;
        String string1 = "tuuid";
        String string2 = "uuid";
        VNodePath inpath = new VBridgePath(string1, string2);
        Node node = NodeCreator.createOFNode(0L);
        NodeConnector nc = NodeConnectorCreator.
            createOFNodeConnector((short)0, node);
        String name = "data";
        VNodePath outpath = new VBridgePath(string1, string2);
        PortLocation inport = new PortLocation(nc, name);
        PortLocation outport = new PortLocation(nc, name);
        DataFlow dataFlow = new DataFlow(ids, created, idle, hard, inpath,
                                         inport, outpath, outport);
        list.add(dataFlow);

        // Single entry.
        List<DataFlow> one = new ArrayList<DataFlow>();
        one.add(dataFlow);
        dflowList = new DataFlowList(one);
        jaxbTest(dflowList, rootName);
        list.add(dataFlow);
        dflowList = new DataFlowList(list);
        jaxbTest(dflowList, rootName);
    }
}
