/*
 * Copyright (c) 2014, 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
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
     * Root XML element name associated with {@link DataFlowList} class.
     */
    private static final String  XML_ROOT = "dataflows";

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
     * Ensure that {@link DataFlowList} is mapped to both XML root element and
     * JSON object.
     */
    @Test
    public void testJAXB() {
        // null list.
        DataFlowList dflowList = new DataFlowList(null);
        String rootName = "dataflows";
        jaxbTest(dflowList, DataFlowList.class, rootName);
        jsonTest(dflowList, DataFlowList.class);

        // Empty list.
        List<DataFlow> list = new ArrayList<DataFlow>();
        dflowList = new DataFlowList(list);
        jaxbTest(dflowList, DataFlowList.class, rootName);
        jsonTest(dflowList, DataFlowList.class);
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
        jaxbTest(dflowList, DataFlowList.class, rootName);
        jsonTest(dflowList, DataFlowList.class);
        list.add(dataFlow);
        dflowList = new DataFlowList(list);
        jaxbTest(dflowList, DataFlowList.class, rootName);
        jsonTest(dflowList, DataFlowList.class);

        // Ensure that broken values in XML can be detected.
        jaxbErrorTest(DataFlowList.class,
                      // DataFlow
                      new XmlAttributeType("dataflow", "id", long.class).
                      add(XML_ROOT),
                      new XmlAttributeType("dataflow", "creationTime",
                                           long.class).
                      add(XML_ROOT),
                      new XmlAttributeType("dataflow", "idleTimeout",
                                           short.class).
                      add(XML_ROOT),
                      new XmlAttributeType("dataflow", "hardTimeout",
                                           short.class).
                      add(XML_ROOT),

                      // FlowMatch
                      new XmlAttributeType("match", "index", Integer.class).
                      add(XML_ROOT, "dataflow"),
                      new XmlAttributeType("ethernet", "type", Integer.class).
                      add(XML_ROOT, "dataflow", "match"),
                      new XmlAttributeType("ethernet", "vlan", Short.class).
                      add(XML_ROOT, "dataflow", "match"),
                      new XmlAttributeType("ethernet", "vlanpri", Byte.class).
                      add(XML_ROOT, "dataflow", "match"),
                      new XmlAttributeType("inet4", "srcsuffix", Short.class).
                      add(XML_ROOT, "dataflow", "match"),
                      new XmlAttributeType("inet4", "dstsuffix", Short.class).
                      add(XML_ROOT, "dataflow", "match"),
                      new XmlAttributeType("inet4", "protocol", Short.class).
                      add(XML_ROOT, "dataflow", "match"),
                      new XmlAttributeType("inet4", "dscp", Byte.class).
                      add(XML_ROOT, "dataflow", "match"),
                      new XmlAttributeType("src", "from", Integer.class).
                      add(XML_ROOT, "dataflow", "match", "tcp"),
                      new XmlAttributeType("src", "to", Integer.class).
                      add(XML_ROOT, "dataflow", "match", "tcp"),
                      new XmlAttributeType("dst", "from", Integer.class).
                      add(XML_ROOT, "dataflow", "match", "tcp"),
                      new XmlAttributeType("dst", "to", Integer.class).
                      add(XML_ROOT, "dataflow", "match", "tcp"),
                      new XmlAttributeType("src", "from", Integer.class).
                      add(XML_ROOT, "dataflow", "match", "udp"),
                      new XmlAttributeType("src", "to", Integer.class).
                      add(XML_ROOT, "dataflow", "match", "udp"),
                      new XmlAttributeType("dst", "from", Integer.class).
                      add(XML_ROOT, "dataflow", "match", "udp"),
                      new XmlAttributeType("dst", "to", Integer.class).
                      add(XML_ROOT, "dataflow", "match", "udp"),
                      new XmlAttributeType("icmp", "type", Short.class).
                      add(XML_ROOT, "dataflow", "match"),
                      new XmlAttributeType("icmp", "code", Short.class).
                      add(XML_ROOT, "dataflow", "match"),

                      // FlowAction
                      new XmlAttributeType("pushvlan", "type", int.class).
                      add(XML_ROOT, "dataflow", "actions"),
                      new XmlAttributeType("vlanid", "vlan", short.class).
                      add(XML_ROOT, "dataflow", "actions"),
                      new XmlAttributeType("vlanpcp", "priority", byte.class).
                      add(XML_ROOT, "dataflow", "actions"),
                      new XmlAttributeType("dscp", "dscp", byte.class).
                      add(XML_ROOT, "dataflow", "actions"),
                      new XmlAttributeType("tpsrc", "port", int.class).
                      add(XML_ROOT, "dataflow", "actions"),
                      new XmlAttributeType("tpdst", "port", int.class).
                      add(XML_ROOT, "dataflow", "actions"),
                      new XmlAttributeType("icmptype", "type", short.class).
                      add(XML_ROOT, "dataflow", "actions"),
                      new XmlAttributeType("icmpcode", "code", short.class).
                      add(XML_ROOT, "dataflow", "actions"),

                      // FlowStats
                      new XmlAttributeType("statistics", "packets", long.class).
                      add(XML_ROOT, "dataflow"),
                      new XmlAttributeType("statistics", "bytes", long.class).
                      add(XML_ROOT, "dataflow"),
                      new XmlAttributeType("statistics", "duration",
                                           long.class).
                      add(XML_ROOT, "dataflow"),

                      // AveragedFlowStats
                      new XmlAttributeType("averagedStats", "packets",
                                           double.class).
                      add(XML_ROOT, "dataflow"),
                      new XmlAttributeType("averagedStats", "bytes",
                                           double.class).
                      add(XML_ROOT, "dataflow"),
                      new XmlAttributeType("averagedStats", "start",
                                           long.class).
                      add(XML_ROOT, "dataflow"),
                      new XmlAttributeType("averagedStats", "end",
                                           long.class).
                      add(XML_ROOT, "dataflow"));
    }
}
