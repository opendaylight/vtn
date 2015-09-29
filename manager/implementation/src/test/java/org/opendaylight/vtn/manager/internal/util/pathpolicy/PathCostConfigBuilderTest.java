/*
 * Copyright (c) 2015 NEC Corporation.  All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.vtn.manager.internal.util.pathpolicy;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import javax.xml.bind.Unmarshaller;

import org.junit.Test;

import org.opendaylight.vtn.manager.PathCost;
import org.opendaylight.vtn.manager.PortLocation;
import org.opendaylight.vtn.manager.SwitchPort;

import org.opendaylight.vtn.manager.internal.util.inventory.SalNode;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcErrorTag;
import org.opendaylight.vtn.manager.internal.util.rpc.RpcException;

import org.opendaylight.vtn.manager.internal.TestBase;

import org.opendaylight.controller.sal.core.Node.NodeIDType;
import org.opendaylight.controller.sal.core.Node;
import org.opendaylight.controller.sal.core.NodeConnector.NodeConnectorIDType;

import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.VtnPathCostConfig;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostList;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.set.path.cost.input.PathCostListBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCost;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.pathpolicy.rev150209.vtn.path.policy.config.VtnPathCostBuilder;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnErrorTag;
import org.opendaylight.yang.gen.v1.urn.opendaylight.vtn.types.rev150209.VtnPortDesc;

/**
 * JUnit test for {@link PathCostConfigBuilder}
 */
public class PathCostConfigBuilderTest extends TestBase {
    /**
     * Test case for all setter methods.
     *
     * <ul>
     *   <li>{@link PathCostConfigBuilder#set(PathCost)}</li>
     *   <li>{@link PathCostConfigBuilder#set(VtnPathCostConfig)}</li>
     *   <li>{@link PathCostConfigBuilder#setPortDesc(VtnPortDesc)}</li>
     *   <li>{@link PathCostConfigBuilder#setCost(Long)}</li>
     *   <li>{@link PathCostConfigBuilder#getBuilder()}</li>
     * </ul>
     *
     * @throws Exception  An error occurred.
     */
    @Test
    public void testSetter() throws Exception {
        // Create a list of valid port locations.
        List<PortLocation> portLocs = new ArrayList<>();
        List<VtnPortDesc> portDescs = new ArrayList<>();

        long[] dpids = {
            Long.MIN_VALUE, -1234567L,  -1L,
            1L, 0xabcdef1234567L, Long.MAX_VALUE,
        };
        String[] ports = {
            null, "1", "10",
        };
        String[] names = {
            null, "port-1", "port-10",
        };

        for (long dpid: dpids) {
            SalNode snode = new SalNode(dpid);
            Node node = snode.getAdNode();
            for (String port: ports) {
                String p;
                String type;
                if (port == null) {
                    p = "";
                    type = null;
                } else {
                    p = port;
                    type = NodeConnectorIDType.OPENFLOW;
                }
                for (String name: names) {
                    String n = (name == null) ? "" : name;
                    String pd = joinStrings(null, null, ",", snode, p, n);
                    VtnPortDesc vdesc = new VtnPortDesc(pd);
                    portDescs.add(vdesc);
                    SwitchPort swport = (port == null && name == null)
                        ? null
                        : new SwitchPort(name, type, port);
                    PortLocation ploc = new PortLocation(node, swport);
                    portLocs.add(ploc);
                }
            }
        }

        Long[] costs = {
            null, Long.valueOf(1L), Long.valueOf(2L), Long.valueOf(100000L),
            Long.valueOf(9999999999L), Long.valueOf(Long.MAX_VALUE - 1L),
            Long.valueOf(Long.MAX_VALUE),
        };

        for (Long cost: costs) {
            Long exCost = cost;
            if (cost != null) {
                Iterator<VtnPortDesc> descIterator = portDescs.iterator();
                for (PortLocation ploc: portLocs) {
                    PathCost pc = new PathCost(ploc, cost.longValue());
                    VtnPortDesc vdesc = descIterator.next();
                    VtnPathCost exVpc = new VtnPathCostBuilder().
                        setPortDesc(vdesc).setCost(cost).build();
                    VtnPathCost vpc = new PathCostConfigBuilder.Data().
                        set(pc).getBuilder().build();
                    assertEquals(exVpc, vpc);

                    PathCostList exPcl = new PathCostListBuilder().
                        setPortDesc(vdesc).setCost(cost).build();
                    PathCostList pcl = new PathCostConfigBuilder.Rpc().
                        set(pc).getBuilder().build();
                    assertEquals(exPcl, pcl);
                }
            } else {
                exCost = Long.valueOf(1L);
            }

            for (VtnPortDesc vdesc: portDescs) {
                VtnPathCostConfig config = new VtnPathCostBuilder().
                    setPortDesc(vdesc).setCost(cost).build();
                VtnPathCost exVpc = new VtnPathCostBuilder().
                    setPortDesc(vdesc).setCost(exCost).build();
                VtnPathCost vpc = new PathCostConfigBuilder.Data().
                    set(config).getBuilder().build();
                assertEquals(exVpc, vpc);
                vpc = new PathCostConfigBuilder.Data().
                    setPortDesc(vdesc).setCost(cost).getBuilder().build();
                assertEquals(exVpc, vpc);

                PathCostList exPcl = new PathCostListBuilder().
                    setPortDesc(vdesc).setCost(exCost).build();
                PathCostList pcl = new PathCostConfigBuilder.Rpc().
                    set(config).getBuilder().build();
                assertEquals(exPcl, pcl);
                pcl = new PathCostConfigBuilder.Rpc().
                    setPortDesc(vdesc).setCost(cost).getBuilder().build();
                assertEquals(exPcl, pcl);
            }
        }

        PathCostConfigBuilder[] pccBuilders = {
            new PathCostConfigBuilder.Data(),
            new PathCostConfigBuilder.Rpc(),
        };

        for (PathCostConfigBuilder pccb: pccBuilders) {
            // Null argument.
            PathCost pc = null;
            try {
                pccb.set(pc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                assertEquals("PathCost cannot be null", e.getMessage());
                assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                assertEquals(null, e.getCause());
            }

            VtnPathCost vpc = null;
            try {
                pccb.set(vpc);
                unexpected();
            } catch (RpcException e) {
                assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                assertEquals("Path cost cannot be null", e.getMessage());
                assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                assertEquals(null, e.getCause());
            }
        }

        // PathCost with null cost.
        Unmarshaller um = createUnmarshaller(PathCost.class);
        StringBuilder sb = new StringBuilder(XML_DECLARATION);
        String xml = sb.append("<pathcost><location><node><type>OF</type>").
            append("<id>00:00:00:00:00:00:00:01</id></node></location>").
            append("</pathcost>").toString();
        PathCost badPc1 = unmarshal(um, xml, PathCost.class);

        // Null port location in PathCost.
        PathCost badPc2 = new PathCost(null, 1L);

        // Invalid AD-SAL node.
        Node[] invalidNodes = {
            null,
            new Node(NodeIDType.ONEPK, "node-1"),
            new Node(NodeIDType.PRODUCTION, "node-2"),
        };

        // Invalid port locations.
        SalNode snode = new SalNode(1L);
        Map<SwitchPort, String> invalidSwPorts = new HashMap<>();
        invalidSwPorts.put(new SwitchPort(NodeConnectorIDType.OPENFLOW, null),
                           "Port type must be specified with port ID");
        invalidSwPorts.put(new SwitchPort((String)null, "1"),
                           "Port ID must be specified with port type");
        invalidSwPorts.put(new SwitchPort(NodeConnectorIDType.ONEPK, "port-1"),
                           "Unsupported node connector type");
        invalidSwPorts.put(new SwitchPort(NodeConnectorIDType.PRODUCTION,
                                          "port-2"),
                           "Unsupported node connector type");
        invalidSwPorts.put(new SwitchPort(NodeConnectorIDType.OPENFLOW,
                                          "port-3"),
                           "Broken node connector is specified");
        invalidSwPorts.put(new SwitchPort(null),
                           "Switch port cannot be empty");
        invalidSwPorts.put(new SwitchPort(""), "Port name cannot be empty");

        // Invalid port descriptors.
        VtnPortDesc[] invalidPortDescs = {
            null,
            new VtnPortDesc("unknown,,"),
            new VtnPortDesc("openflow:badnode,,"),
            new VtnPortDesc("openflow1:2,,"),
        };

        // Invalid link costs.
        long[] invalidCosts = {
            Long.MIN_VALUE, -12345678L, -10L, -3L, -2L, -1L, 0L,
        };

        PortLocation ploc = new PortLocation(snode.getAdNode(), null);

        for (PathCostConfigBuilder pccb: pccBuilders) {
            try {
                pccb.set(badPc1);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                assertEquals("Cost in PathCost cannot be null",
                             e.getMessage());
            }

            try {
                pccb.set(badPc2);
                unexpected();
            } catch (RpcException e) {
                assertEquals(RpcErrorTag.MISSING_ELEMENT, e.getErrorTag());
                assertEquals(null, e.getCause());
                assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                assertEquals("Port location cannot be null", e.getMessage());
            }

            for (Node node: invalidNodes) {
                PortLocation badPloc = new PortLocation(node, null);
                PathCost pc = new PathCost(badPloc, 1L);
                try {
                    pccb.set(pc);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(null, e.getCause());
                    assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                    if (node == null) {
                        assertEquals("Node cannot be null", e.getMessage());
                        assertEquals(RpcErrorTag.MISSING_ELEMENT,
                                     e.getErrorTag());
                    } else {
                        String msg = "Unsupported node: type=" + node.getType();
                        assertEquals(msg, e.getMessage());
                        assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    }
                }
            }

            for (Map.Entry<SwitchPort, String> entry:
                     invalidSwPorts.entrySet()) {
                SwitchPort swport = entry.getKey();
                String msg = entry.getValue();
                PortLocation badPloc =
                    new PortLocation(snode.getAdNode(), swport);
                PathCost pc = new PathCost(badPloc, 1L);
                try {
                    pccb.set(pc);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    assertEquals(null, e.getCause());
                    assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                    assertEquals(msg, e.getMessage());
                }
            }

            for (VtnPortDesc vdesc: invalidPortDescs) {
                VtnPathCost config = new VtnPathCostBuilder().
                    setPortDesc(vdesc).build();
                try {
                    pccb.set(config);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                    if (vdesc == null) {
                        assertEquals("vtn-port-desc cannot be null",
                                     e.getMessage());
                        assertEquals(RpcErrorTag.MISSING_ELEMENT,
                                     e.getErrorTag());
                    } else {
                        String msg = "Invalid vtn-port-desc: " +
                            vdesc.getValue() + ": ";
                        assertTrue(e.getMessage().startsWith(msg));
                        assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    }
                }

                try {
                    pccb.setPortDesc(vdesc);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                    if (vdesc == null) {
                        assertEquals("vtn-port-desc cannot be null",
                                     e.getMessage());
                        assertEquals(RpcErrorTag.MISSING_ELEMENT,
                                     e.getErrorTag());
                    } else {
                        String msg = "Invalid vtn-port-desc: " +
                            vdesc.getValue() + ": ";
                        assertTrue(e.getMessage().startsWith(msg));
                        assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    }
                }
            }

            for (long c: invalidCosts) {
                PathCost pc = new PathCost(ploc, c);
                try {
                    pccb.set(pc);
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                    String msg = "Invalid cost value: " + c;
                    assertEquals(msg, e.getMessage());
                    Throwable t = e.getCause();
                    assertTrue("Unexpected cause: " + t,
                               t instanceof IllegalArgumentException);
                }

                try {
                    pccb.setCost(Long.valueOf(c));
                    unexpected();
                } catch (RpcException e) {
                    assertEquals(RpcErrorTag.BAD_ELEMENT, e.getErrorTag());
                    assertEquals(VtnErrorTag.BADREQUEST, e.getVtnErrorTag());
                    String msg = "Invalid cost value: " + c;
                    assertEquals(msg, e.getMessage());
                    Throwable t = e.getCause();
                    assertTrue("Unexpected cause: " + t,
                               t instanceof IllegalArgumentException);
                }
            }
        }
    }
}
